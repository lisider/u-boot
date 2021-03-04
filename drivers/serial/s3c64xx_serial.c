// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 *
 * based on drivers/serial/s3c64xx.c
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/uart.h>
#include <serial.h>
#include <clk.h>

DECLARE_GLOBAL_DATA_PTR;

#define RX_FIFO_COUNT_SHIFT	0
#define RX_FIFO_COUNT_MASK	(0x3f << RX_FIFO_COUNT_SHIFT)
#define RX_FIFO_FULL		(1 << 6)
#define TX_FIFO_COUNT_SHIFT	8
#define TX_FIFO_COUNT_MASK	(0x3f << TX_FIFO_COUNT_SHIFT)
#define TX_FIFO_FULL		(1 << 14)

/* Information about a serial port */
struct s3c64xx_serial_platdata {
	struct s3c64xx_uart *reg;  /* address of registers in physical memory */
	u32 port_reg_size; /* size for port register range */
	u8 port_id;     /* uart port number */
};

/*
 * The coefficient, used to calculate the baudrate on S3C6400 UARTs is
 * calculated as
 * C = UBRDIV * 16 + number_of_set_bits_in_UDIVSLOT
 * however, section 31.6.11 of the datasheet doesn't recomment using 1 for 1,
 * 3 for 2, ... (2^n - 1) for n, instead, they suggest using these constants:
 */
static const int udivslot[] = {
	0,
	0x0080,
	0x0808,
	0x0888,
	0x2222,
	0x4924,
	0x4a52,
	0x54aa,
	0x5555,
	0xd555,
	0xd5d5,
	0xddd5,
	0xdddd,
	0xdfdd,
	0xdfdf,
	0xffdf,
};

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int __maybe_unused s3c64xx_serial_init(struct s3c64xx_uart *uart)
{
	/* reset and enable FIFOs, set triggers to the maximum */
	uart->UFCON = 0;//0xff;
	uart->UMCON = 0;
	/* 8N1 */
	uart->ULCON = 3;
	/* No interrupts, no DMA, pure polling */
#ifdef CFG_CLKSRC_CLKUART
	uart->UCON =0xe45;		/* UARTCLK SRC = 11 => EXT_UCLK1*/
#else
	uart->UCON =0xa45;		/* UARTCLK SRC = 10 => PCLK */
#endif
	//serial_setbrg();

	return 0;
}

static void __maybe_unused s3c64xx_serial_baud(struct s3c64xx_uart *uart, uint uclk,
					   int baudrate)
{
	u32 val;

	val = (uclk / baudrate) % 16;

	uart->UBRDIV = uclk / baudrate / 16 - 1;
	uart->UDIVSLOT = udivslot[val];

	for (val = 0; val < 100; val++)
		barrier();
}


static int __maybe_unused s3c64xx_serial_err_check(const struct s3c64xx_uart *const uart, int op)
{
	unsigned int mask;

	/*
	 * UERSTAT
	 * Break Detect	[3]
	 * Frame Err	[2] : receive operation
	 * Parity Err	[1] : receive operation
	 * Overrun Err	[0] : receive operation
	 */
	if (op)
		mask = 0x8;
	else
		mask = 0xf;

	return uart->UERSTAT & mask;
}

#if !defined(CONFIG_SPL_BUILD)  || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
#if CONFIG_IS_ENABLED(DM_SERIAL)
int s3c64xx_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct s3c64xx_serial_platdata *plat = dev->platdata;
	struct s3c64xx_uart *const uart = plat->reg;
	u32 uclk;

	uclk = get_uart_clk(plat->port_id);

	s3c64xx_serial_baud(uart, uclk, baudrate);

	return 0;
}

static int s3c64xx_serial_probe(struct udevice *dev)
{
	struct s3c64xx_serial_platdata *plat = dev->platdata;
	struct s3c64xx_uart *const uart = plat->reg;

	s3c64xx_serial_init(uart);

	return 0;
}

static int s3c64xx_serial_getc(struct udevice *dev)
{
	struct s3c64xx_serial_platdata *plat = dev->platdata;
	struct s3c64xx_uart *const uart = plat->reg;

//	if (!(uart->UFSTAT) & RX_FIFO_COUNT_MASK))
//		return -EAGAIN;

	/* wait for character to arrive */
	while (!(uart->UTRSTAT & 0x1));

	s3c64xx_serial_err_check(uart, 0);
	return uart->URXH & 0xff;
}

static int s3c64xx_serial_putc(struct udevice *dev, const char ch)
{
	struct s3c64xx_serial_platdata *plat = dev->platdata;
	struct s3c64xx_uart *const uart = plat->reg;

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

	uart->UTXH = ch;

	s3c64xx_serial_err_check(uart, 1);

	/* If \n, also do \r */
	if (ch == '\n')
		serial_putc('\r');
	return 0;
}

static int s3c64xx_serial_pending(struct udevice *dev, bool input)
{
	struct s3c64xx_serial_platdata *plat = dev->platdata;
	struct s3c64xx_uart *const uart = plat->reg;
	uint32_t ufstat = uart->UFSTAT;

	if (input)
		return (ufstat & RX_FIFO_COUNT_MASK) >> RX_FIFO_COUNT_SHIFT;
	else
		return (ufstat & TX_FIFO_COUNT_MASK) >> TX_FIFO_COUNT_SHIFT;
}

static int s3c64xx_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct s3c64xx_serial_platdata *plat = dev->platdata;
	fdt_addr_t addr,size;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->port_id = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"id", dev->seq);
	devfdt_get_addr_size_index(dev, 0, &size);
	plat->port_reg_size = size;
	plat->reg = (struct s3c64xx_uart *)(addr + plat->port_id * size);
	return 0;
}

static const struct dm_serial_ops s3c64xx_serial_ops = {
	.putc = s3c64xx_serial_putc,
	.pending = s3c64xx_serial_pending,
	.getc = s3c64xx_serial_getc,
	.setbrg = s3c64xx_serial_setbrg,
};

static const struct udevice_id s3c64xx_serial_ids[] = {
	{ .compatible = "samsung,ok6410-uart" },
	{ }
};

U_BOOT_DRIVER(serial_s3c64xx) = {
	.name	= "serial_s3c64xx",
	.id	= UCLASS_SERIAL,
	.of_match = s3c64xx_serial_ids,
	.ofdata_to_platdata = s3c64xx_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct s3c64xx_serial_platdata),
	.probe = s3c64xx_serial_probe,
	.ops	= &s3c64xx_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

#else /* CONFIG_SPL_BUILD  and ! CONFIG_IS_ENABLED(DM_SERIAL) */

static void s3c_serial_setbrg_internal(struct s3c64xx_uart *const uart, int id,
					 int baudrate)
{
	u32 uclk = get_uart_clk(id);

	s3c64xx_serial_baud(uart, uclk, baudrate);
}

static void s3c_serial_setbrg(void)
{
	s3c_serial_setbrg_internal((struct s3c64xx_uart * )CONFIG_DEBUG_UART_BASE,
				     0 /*CFG_SERIAL_ID*/,CONFIG_BAUDRATE);
}

static int s3c_serial_init(void)
{
	struct s3c64xx_uart *const usart = (struct s3c64xx_uart * )CONFIG_DEBUG_UART_BASE;

	s3c64xx_serial_init(usart);
	s3c_serial_setbrg();

	return 0;
}

static void s3c_serial_putc(char ch)
{
	struct s3c64xx_uart *const uart = (struct s3c64xx_uart * )CONFIG_DEBUG_UART_BASE;

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

	uart->UTXH = ch;

	s3c64xx_serial_err_check(uart, 1);

	/* If \n, also do \r */
	if (ch == '\n')
		serial_putc('\r');
}

static int s3c_serial_getc(void)
{
	struct s3c64xx_uart *const uart = (struct s3c64xx_uart * )CONFIG_DEBUG_UART_BASE;

	/* wait for character to arrive */
	while (!(uart->UTRSTAT & 0x1));

	s3c64xx_serial_err_check(uart, 0);
	return uart->URXH & 0xff;
}

static int s3c_serial_tstc(void)
{
	struct s3c64xx_uart *const uart = (struct s3c64xx_uart * )CONFIG_DEBUG_UART_BASE;
	return (uart->UTRSTAT & 0x1) != 0;
}

static struct serial_device s3c64xx_serial_drv = {
	.name	= "s3c64xx_serial",
	.start	= s3c_serial_init,
	.stop	= NULL,
	.setbrg	= s3c_serial_setbrg,
	.putc	= s3c_serial_putc,
	.puts	= default_serial_puts,
	.getc	= s3c_serial_getc,
	.tstc 	= s3c_serial_tstc,
};

void s3c_serial_initialize(void)
{
	serial_register(&s3c64xx_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &s3c64xx_serial_drv;
}
#endif /* CONFIG_IS_ENABLED(DM_SERIAL) */
#endif /* !CONFIG_SPL_BUILD || CONFIG_SPL_LIBCOMMON_SUPPORT */

#if defined(CONFIG_DEBUG_UART_S3C) || !defined(CONFIG_SPL_LIBCOMMON_SUPPORT)

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct s3c64xx_uart *uart = (struct s3c64xx_uart *)CONFIG_DEBUG_UART_BASE;

	s3c64xx_serial_init(uart);
	s3c64xx_serial_baud(uart, CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
}

static inline void _debug_uart_putc(int ch)
{
	struct s3c64xx_uart *uart = (struct s3c64xx_uart *)CONFIG_DEBUG_UART_BASE;

	while (uart->UFSTAT & TX_FIFO_FULL);

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

	writeb(ch, &uart->UTXH);
}
#if 0
void s3c_printch(int ch)
{
	if (ch == '\n')
		_debug_uart_putc('\r');
	_debug_uart_putc(ch);
}
void s3c_printascii(const char *str)
{
	while (*str) 
		s3c_printch(*str++);
}
#endif
DEBUG_UART_FUNCS

void __weak puts(const char *s)
{
	while (*s) {
		int ch = *s++;

		printch(ch);
	}
	return;
}

int __weak printf(const char *fmt, ...)
{
	puts(fmt);
	return 0;
}
#endif
