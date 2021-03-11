/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sdhc.h>
#include <debug_uart.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
#define CS8900_Tacs	(0x0)	// 0clk		address set-up
#define CS8900_Tcos	(0x4)	// 4clk		chip selection set-up
#define CS8900_Tacc	(0xE)	// 14clk	access cycle
#define CS8900_Tcoh	(0x1)	// 1clk		chip selection hold
#define CS8900_Tah	(0x4)	// 4clk		address holding time
#define CS8900_Tacp	(0x6)	// 6clk		page mode access cycle
#define CS8900_PMC	(0x0)	// normal(1data)page mode configuration

/*
 * Miscellaneous platform dependent initialisations
 */

static void cs8900_pre_init(void)
{
	SROM_BW_REG &= ~(0xf << 4);
	SROM_BW_REG |= (1 << 7) | (1 << 6) | (1 << 4);
	SROM_BC1_REG = ((CS8900_Tacs << 28) + (CS8900_Tcos << 24) +
					(CS8900_Tacc << 16) + (CS8900_Tcoh << 12) +
					(CS8900_Tah << 8) + (CS8900_Tacp << 4) + CS8900_PMC);
}

int board_init(void)
{
	cs8900_pre_init();

	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1+0x100);

	return 0;
}

int dram_init(void)
{
	unsigned int i; 

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		//addr = CONFIG_SYS_SDRAM_BASE + (i * SDRAM_BANK_SIZE);
		gd->ram_size += SDRAM_BANK_SIZE;
	}
	//debug("size %d MB, base at %#x\n",(int)(gd->ram_size>>20),CONFIG_SYS_SDRAM_BASE);
	return 0;
}

int dram_init_banksize(void)
{
	unsigned int i;
	unsigned long addr;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		addr = CONFIG_SYS_SDRAM_BASE + (i * SDRAM_BANK_SIZE);
		//size = get_ram_size((long *)addr, SDRAM_BANK_SIZE);

		gd->bd->bi_dram[i].start = addr;
		gd->bd->bi_dram[i].size = SDRAM_BANK_SIZE;
	}

	return 0;
}

#ifdef BOARD_LATE_INIT
#if defined(CONFIG_BOOT_NAND)
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[100];

	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "nand erase 0 40000;nand write %08x 0 40000", PHYS_SDRAM_1 + 0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\nready for self-burning U-Boot image\n\n");
		env_set("bootdelay", "0");
		env_set("bootcmd", boot_cmd);
	}

	return 0;
}
#elif defined(CONFIG_BOOT_MOVINAND)
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[100];
	int hc;

	hc = (magic[2] & 0x1) ? 1 : 0;

	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "movi init %d %d;movi write u-boot %08x", magic[3], hc, PHYS_SDRAM_1 + 0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\nready for self-burning U-Boot image\n\n");
		env_set("bootdelay", "0");
		env_set("bootcmd", boot_cmd);
	}

	return 0;
}
#else
int board_late_init (void)
{
	return 0;
}
#endif
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board:   OK6410A\n");
	return (0);
}
#endif

#if defined(CONFIG_MMC) || defined(CONFIG_SPL_MMC_SUPPORT)
int s3c_sdhc_pin_config(int peripheral, int flags);
int board_mmc_init(struct bd_info *bis)
{
	int err;

	err = s3c_sdhc_pin_config(PERIPH_ID_SDMMC0, 0);

	if( !err)
		err = s3c_mmc_init(PERIPH_ID_SDMMC0, 4);

	return err;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	//int err = board_uart_init();
 #ifdef CONFIG_DEBUG_UART
	 debug_uart_init();
 #endif
	return 0;
}

#endif

#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(struct bd_info *bis)
{
	return dm9000_initialize(bis);

}
#endif

#ifdef CONFIG_SPL_BUILD
#include <asm/spl.h>
#include <asm/arch/clock-regs.h>

extern ulong _UBRDIV;
extern ulong _UDIVSLOT;
extern void spl_boot_mmc(void);
extern void spl_boot_nand(void);

static ulong spl_uart_getclk(int is_mpll)
{
	ulong r, m, p, s;
	ulong uartclk;

	r =(is_mpll) ? MPLL_CON_REG : EPLL_CON0_REG;
	m = (r>>16) & 0x3ff;
	p = (r>>8) & 0x3f;
	s = r & 0x7;
	
	uartclk = (m * (CONFIG_SYS_CLK_FREQ / (p << s)));
	if (is_mpll && 
		(CLK_DIV0_REG & S3C64XX_CLKDIV0_MPLL_MASK))
		uartclk /=2;
	return uartclk;
}

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

void spl_config_uart_baudrate(int is_mpll)
{
#define BAUD_RATE 115200
	/* use extclk as uart clock */
	ulong val = spl_uart_getclk(is_mpll) / BAUD_RATE;

	_UBRDIV = val / 16 - 1;
	_UDIVSLOT = udivslot[val % 16];
}
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

__weak  void hang(void)
{
	for(;;)  ;
}

void spl_board_init(void)
{
#ifdef CONFIG_SPL_MMC_BOOT
	spl_boot_mmc();
#endif //CONFIG_SPL_MMC_BOOT

#ifdef CONFIG_SPL_NAND_BOOT
	spl_boot_nand();
#endif //CONFIG_SPL_NAND_BOOT
}

void  board_init_f(ulong dummy)
{
	//dcache_enable();
#ifndef CONFIG_SPL_LIBCOMMON_SUPPORT
	/* Uart debug for sure */
	debug_uart_init();
#endif
}

#endif  /* CONFIG_SPL_BUILD */
