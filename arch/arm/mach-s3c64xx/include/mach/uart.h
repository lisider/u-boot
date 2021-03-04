#ifndef __S3C64XX_UART_H__
#define __S3C64XX_UART_H__

#define S3C64XX_UART_CHANNELS   3

#ifndef __ASSEMBLY__
typedef enum {
    S3C64XX_UART0,
    S3C64XX_UART1,
    S3C64XX_UART2,
    S3C64XX_UART3,
} S3C64XX_UARTS_NR;

/* UART (see manual chapter 11) */
typedef struct s3c64xx_uart{
    S3C64XX_REG32   ULCON;
    S3C64XX_REG32   UCON;
    S3C64XX_REG32   UFCON;
    S3C64XX_REG32   UMCON;
    S3C64XX_REG32   UTRSTAT;
    S3C64XX_REG32   UERSTAT;
    S3C64XX_REG32   UFSTAT;
    S3C64XX_REG32   UMSTAT;
#ifdef __BIG_ENDIAN
    S3C64XX_REG8    res1[3];
    S3C64XX_REG8    UTXH;
    S3C64XX_REG8    res2[3];
    S3C64XX_REG8    URXH;
#else /* Little Endian */
    S3C64XX_REG8    UTXH;
    S3C64XX_REG8    res1[3];
    S3C64XX_REG8    URXH;
    S3C64XX_REG8    res2[3];
#endif
    S3C64XX_REG32   UBRDIV;
	S3C64XX_REG32   UDIVSLOT;
} /*__attribute__((__packed__))*/ S3C64XX_UART;

#endif


/* 
 * UART register I/O 
 */
#define rULCON0         (*(volatile unsigned *)0x50000000)
#define rUCON0          (*(volatile unsigned *)0x50000004)
#define rUFCON0         (*(volatile unsigned *)0x50000008)
#define rUMCON0         (*(volatile unsigned *)0x5000000C)
#define rUTRSTAT0       (*(volatile unsigned *)0x50000010)
#define rUERSTAT0       (*(volatile unsigned *)0x50000014)
#define rUFSTAT0        (*(volatile unsigned *)0x50000018)
#define rUMSTAT0        (*(volatile unsigned *)0x5000001C)
#define rUBRDIV0        (*(volatile unsigned *)0x50000028)

#define rULCON1         (*(volatile unsigned *)0x50004000)
#define rUCON1          (*(volatile unsigned *)0x50004004)
#define rUFCON1         (*(volatile unsigned *)0x50004008)
#define rUMCON1         (*(volatile unsigned *)0x5000400C)
#define rUTRSTAT1       (*(volatile unsigned *)0x50004010)
#define rUERSTAT1       (*(volatile unsigned *)0x50004014)
#define rUFSTAT1        (*(volatile unsigned *)0x50004018)
#define rUMSTAT1        (*(volatile unsigned *)0x5000401C)
#define rUBRDIV1        (*(volatile unsigned *)0x50004028)

#define rULCON2         (*(volatile unsigned *)0x50008000)
#define rUCON2          (*(volatile unsigned *)0x50008004)
#define rUFCON2         (*(volatile unsigned *)0x50008008)
#define rUTRSTAT2       (*(volatile unsigned *)0x50008010)
#define rUERSTAT2       (*(volatile unsigned *)0x50008014)
#define rUFSTAT2        (*(volatile unsigned *)0x50008018)
#define rUBRDIV2        (*(volatile unsigned *)0x50008028)

#ifdef __BIG_ENDIAN
#define rUTXH0          (*(volatile unsigned char *)0x50000023)
#define rURXH0          (*(volatile unsigned char *)0x50000027)
#define rUTXH1          (*(volatile unsigned char *)0x50004023)
#define rURXH1          (*(volatile unsigned char *)0x50004027)
#define rUTXH2          (*(volatile unsigned char *)0x50008023)
#define rURXH2          (*(volatile unsigned char *)0x50008027)

#define WrUTXH0(ch)     (*(volatile unsigned char *)0x50000023)=(unsigned char)(ch)
#define RdURXH0()       (*(volatile unsigned char *)0x50000027)
#define WrUTXH1(ch)     (*(volatile unsigned char *)0x50004023)=(unsigned char)(ch)
#define RdURXH1()       (*(volatile unsigned char *)0x50004027)
#define WrUTXH2(ch)     (*(volatile unsigned char *)0x50008023)=(unsigned char)(ch)
#define RdURXH2()       (*(volatile unsigned char *)0x50008027)
#define UTXH0           (0x50000020+3)  /* byte_access address by DMA */
#define URXH0           (0x50000024+3)
#define UTXH1           (0x50004020+3)
#define URXH1           (0x50004024+3)
#define UTXH2           (0x50008020+3)
#define URXH2           (0x50008024+3)

#else /* Little Endian */
#define rUTXH0          (*(volatile unsigned char *)0x50000020)
#define rURXH0          (*(volatile unsigned char *)0x50000024)
#define rUTXH1          (*(volatile unsigned char *)0x50004020)
#define rURXH1          (*(volatile unsigned char *)0x50004024)
#define rUTXH2          (*(volatile unsigned char *)0x50008020)
#define rURXH2          (*(volatile unsigned char *)0x50008024)

#define WrUTXH0(ch)     (*(volatile unsigned char *)0x50000020)=(unsigned char)(ch)
#define RdURXH0()       (*(volatile unsigned char *)0x50000024)
#define WrUTXH1(ch)     (*(volatile unsigned char *)0x50004020)=(unsigned char)(ch)
#define RdURXH1()       (*(volatile unsigned char *)0x50004024)
#define WrUTXH2(ch)     (*(volatile unsigned char *)0x50008020)=(unsigned char)(ch)
#define RdURXH2()       (*(volatile unsigned char *)0x50008024)

#define UTXH0           (0x50000020)    /* byte_access address by DMA */
#define URXH0           (0x50000024)
#define UTXH1           (0x50004020)
#define URXH1           (0x50004024)
#define UTXH2           (0x50008020)
#define URXH2           (0x50008024)
#endif

/*
 * UART register address
 */
#define ELFIN_UART_BASE     0x7F005000

#define ELFIN_UART0_OFFSET  0x0000
#define ELFIN_UART1_OFFSET  0x0400
#define ELFIN_UART2_OFFSET  0x0800
#define ELFIN_UART3_OFFSET  0x0c00

#ifdef CONFIG_SERIAL1
#define ELFIN_UART_CONSOLE_BASE (ELFIN_UART_BASE + ELFIN_UART0_OFFSET)
#elif defined(CONFIG_SERIAL2)
#define ELFIN_UART_CONSOLE_BASE (ELFIN_UART_BASE + ELFIN_UART1_OFFSET)
#elif defined(CONFIG_SERIAL3)
#define ELFIN_UART_CONSOLE_BASE (ELFIN_UART_BASE + ELFIN_UART2_OFFSET)
#elif defined(CONFIG_SERIAL4)
#define ELFIN_UART_CONSOLE_BASE (ELFIN_UART_BASE + ELFIN_UART3_OFFSET)
#else
#define ELFIN_UART_CONSOLE_BASE (ELFIN_UART_BASE + ELFIN_UART0_OFFSET)
#endif

#define ULCON_OFFSET        0x00
#define UCON_OFFSET     0x04
#define UFCON_OFFSET        0x08
#define UMCON_OFFSET        0x0C
#define UTRSTAT_OFFSET      0x10
#define UERSTAT_OFFSET      0x14
#define UFSTAT_OFFSET       0x18
#define UMSTAT_OFFSET       0x1C
#define UTXH_OFFSET     0x20
#define URXH_OFFSET     0x24
#define UBRDIV_OFFSET       0x28
#define UDIVSLOT_OFFSET     0x2C
#define UINTP_OFFSET        0x30
#define UINTSP_OFFSET       0x34
#define UINTM_OFFSET        0x38

#define ULCON0_REG      __REG(0x7F005000)
#define UCON0_REG       __REG(0x7F005004)
#define UFCON0_REG      __REG(0x7F005008)
#define UMCON0_REG      __REG(0x7F00500C)
#define UTRSTAT0_REG        __REG(0x7F005010)
#define UERSTAT0_REG        __REG(0x7F005014)
#define UFSTAT0_REG     __REG(0x7F005018)
#define UMSTAT0_REG     __REG(0x7F00501c)
#define UTXH0_REG       __REG(0x7F005020)
#define URXH0_REG       __REG(0x7F005024)
#define UBRDIV0_REG     __REG(0x7F005028)
#define UDIVSLOT0_REG       __REG(0x7F00502c)
#define UINTP0_REG      __REG(0x7F005030)
#define UINTSP0_REG     __REG(0x7F005034)
#define UINTM0_REG      __REG(0x7F005038)

#define ULCON1_REG      __REG(0x7F005400)
#define UCON1_REG       __REG(0x7F005404)
#define UFCON1_REG      __REG(0x7F005408)
#define UMCON1_REG      __REG(0x7F00540C)
#define UTRSTAT1_REG        __REG(0x7F005410)
#define UERSTAT1_REG        __REG(0x7F005414)
#define UFSTAT1_REG     __REG(0x7F005418)
#define UMSTAT1_REG     __REG(0x7F00541c)
#define UTXH1_REG       __REG(0x7F005420)
#define URXH1_REG       __REG(0x7F005424)
#define UBRDIV1_REG     __REG(0x7F005428)
#define UDIVSLOT1_REG       __REG(0x7F00542c)
#define UINTP1_REG      __REG(0x7F005430)
#define UINTSP1_REG     __REG(0x7F005434)
#define UINTM1_REG      __REG(0x7F005438)

#define UTRSTAT_TX_EMPTY    BIT2
#define UTRSTAT_RX_READY    BIT0
#define UART_ERR_MASK       0xF

#ifndef __ASSEMBLY__
static inline S3C64XX_UART * S3C64XX_GetBase_UART(S3C64XX_UARTS_NR nr)
{
//  return (S3C64XX_UART *)(ELFIN_UART_BASE + (nr * 0x4000));
    return (S3C64XX_UART *)(ELFIN_UART_BASE + (nr*0x400));
}
#endif

#endif
