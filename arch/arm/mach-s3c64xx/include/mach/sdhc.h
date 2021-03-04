#ifndef __S3C64XX_SDHC_H__
#define __S3C64XX_SDHC_H__

#include "types.h"

/*
 * HS MMC Interface
 */
#define ELFIN_HSMMC_BASE	0x7C200000
#define ELFIN_HSMMC_0_BASE	0x7c200000
#define ELFIN_HSMMC_1_BASE	0x7c300000
#define ELFIN_HSMMC_2_BASE	0x7c400000
#define HSMMC_REG_OFFSET   0x100000

#define HM_SYSAD		(0x00)
#define HM_BLKSIZE		(0x04)
#define HM_BLKCNT		(0x06)
#define HM_ARGUMENT		(0x08)
#define HM_TRNMOD		(0x0c)
#define HM_CMDREG		(0x0e)
#define HM_RSPREG0		(0x10)
#define HM_RSPREG1		(0x14)
#define HM_RSPREG2		(0x18)
#define HM_RSPREG3		(0x1c)
#define HM_BDATA		(0x20)
#define HM_PRNSTS		(0x24)
#define HM_HOSTCTL		(0x28)
#define HM_PWRCON		(0x29)
#define HM_BLKGAP		(0x2a)
#define HM_WAKCON		(0x2b)
#define HM_CLKCON		(0x2c)
#define HM_TIMEOUTCON		(0x2e)
#define HM_SWRST		(0x2f)
#define HM_NORINTSTS		(0x30)
#define HM_ERRINTSTS		(0x32)
#define HM_NORINTSTSEN		(0x34)
#define HM_ERRINTSTSEN		(0x36)
#define HM_NORINTSIGEN		(0x38)
#define HM_ERRINTSIGEN		(0x3a)
#define HM_ACMD12ERRSTS		(0x3c)
#define HM_CAPAREG		(0x40)
#define HM_MAXCURR		(0x48)
#define HM_CONTROL2		(0x80)
#define HM_CONTROL3		(0x84)
#define HM_CONTROL4		(0x8c)
#define HM_HCVER		(0xfe)

#define HM_CTRL2_ENSTAASYNCCLR	(1 << 31)
#define HM_CTRL2_ENCMDCNFMSK		(1 << 30)
#define HM_CTRL2_CDINVRXD3		(1 << 29)
#define HM_CTRL2_SLCARDOUT		(1 << 28)

#define HM_CTRL2_FLTCLKSEL_MASK	(0xf << 24)
#define HM_CTRL2_FLTCLKSEL_SHIFT	(24)
#define HM_CTRL2_FLTCLKSEL(_x)	((_x) << 24)

#define HM_CTRL2_LVLDAT_MASK		(0xff << 16)
#define HM_CTRL2_LVLDAT_SHIFT	(16)
#define HM_CTRL2_LVLDAT(_x)		((_x) << 16)

#define HM_CTRL2_ENFBCLKTX		(1 << 15)
#define HM_CTRL2_ENFBCLKRX		(1 << 14)
#define HM_CTRL2_SDCDSEL		(1 << 13)
#define HM_CTRL2_SDSIGPC		(1 << 12)
#define HM_CTRL2_ENBUSYCHKTXSTART	(1 << 11)

#define HM_CTRL2_DFCNT_MASK(_x)	((_x) << 9)
#define HM_CTRL2_DFCNT_SHIFT		(9)

#define HM_CTRL2_ENCLKOUTHOLD	(1 << 8)
#define HM_CTRL2_RWAITMODE		(1 << 7)
#define HM_CTRL2_DISBUFRD		(1 << 6)
#define HM_CTRL2_SELBASECLK_MASK(_x)	((_x) << 4)
#define HM_CTRL2_SELBASECLK_SHIFT	(4)
#define HM_CTRL2_PWRSYNC		(1 << 3)
#define HM_CTRL2_ENCLKOUTMSKCON	(1 << 1)
#define HM_CTRL2_HWINITFIN		(1 << 0)

#define HM_CTRL3_FCSEL3		(1 << 31)
#define HM_CTRL3_FCSEL2		(1 << 23)
#define HM_CTRL3_FCSEL1		(1 << 15)
#define HM_CTRL3_FCSEL0		(1 << 7)

#define HM_CTRL4_DRIVE_MASK(_x)	((_x) << 16)
#define HM_CTRL4_DRIVE_SHIFT		(16)

#ifndef __ASSEMBLY__

#include <asm/arch/periph.h>

#define S3C_SDHCI_MAX_HOSTS 3

/* MMC INTERFACE (see S3C6410 manual chapter 27) */
typedef struct {
	S3C64XX_REG32 sdma_addr;  /* offset + 0x00 */
	S3C64XX_REG16 blksize;
	S3C64XX_REG16 blkcnt;
	S3C64XX_REG32 argument;
	S3C64XX_REG16 tranmode;
	S3C64XX_REG16 cmdreg;
	S3C64XX_REG32 respreg0;    /* offset + 0x10 */
	S3C64XX_REG32 respreg1;
	S3C64XX_REG32 respreg2;
	S3C64XX_REG32 respreg3;
	S3C64XX_REG32 bufdata;     /* offset + 0x20 */
	S3C64XX_REG32 preststat;
	S3C64XX_REG8  hostctl;
	S3C64XX_REG8  pwrcon;
	S3C64XX_REG8  blkgap;
	S3C64XX_REG8  wakecon;
	S3C64XX_REG16 clkcon;
	S3C64XX_REG8  timeoutcon;
	S3C64XX_REG8  swrst;
	S3C64XX_REG16 norintstatus; /* offset + 0x30 */
	S3C64XX_REG16 errintstauts;
	S3C64XX_REG16 norintstatus_en;
	S3C64XX_REG16 errintstauts_en;
	S3C64XX_REG16 norintsignal_en;
	S3C64XX_REG16 errintsignal_en;
	S3C64XX_REG16 acmd12errstatus;
	S3C64XX_REG16 res0;
	S3C64XX_REG32 capreg;     /* offset + 0x40 */
	S3C64XX_REG32 res1;
	S3C64XX_REG32 maxcurr;
	S3C64XX_REG32 res2;
	S3C64XX_REG16 fea_errint;  /* offset + 0x50 */
	S3C64XX_REG16 fe_errint;
	S3C64XX_REG32 admaerrstatus;
	S3C64XX_REG32 admasysaddr;
	S3C64XX_REG32 res3;
	S3C64XX_REG32 res4[8];    /* offset + 0x60 */
	S3C64XX_REG32 control2;   /* offset + 0x80 */
	S3C64XX_REG32 control3;
	S3C64XX_REG32 res5;
	S3C64XX_REG32 control4;
	S3C64XX_REG32 res6[24];  /* offset + 0x90 */
	S3C64XX_REG16 res7[7] ; /* offset + 0xF0 */
	S3C64XX_REG16 hostctrlversion;
} /*__attribute__((__packed__))*/ S3C6410_SDMMC;


/* SD INTERFACE (see S3C2410 manual chapter 19) */
typedef struct {
	S3C64XX_REG32	SDICON;
	S3C64XX_REG32	SDIPRE;
	S3C64XX_REG32	SDICARG;
	S3C64XX_REG32	SDICCON;
	S3C64XX_REG32	SDICSTA;
	S3C64XX_REG32	SDIRSP0;
	S3C64XX_REG32	SDIRSP1;
	S3C64XX_REG32	SDIRSP2;
	S3C64XX_REG32	SDIRSP3;
	S3C64XX_REG32	SDIDTIMER;
	S3C64XX_REG32	SDIBSIZE;
	S3C64XX_REG32	SDIDCON;
	S3C64XX_REG32	SDIDCNT;
	S3C64XX_REG32	SDIDSTA;
	S3C64XX_REG32	SDIFSTA;
#ifdef __BIG_ENDIAN
	S3C64XX_REG8	res[3];
	S3C64XX_REG8	SDIDAT;
#else
	S3C64XX_REG8	SDIDAT;
	S3C64XX_REG8	res[3];
#endif
	S3C64XX_REG32	SDIIMSK;
} /*__attribute__((__packed__))*/ S3C2410_SDI;

int s3c_sdhci_init(u32 regbase, int index, int bus_width);

static inline int s3c_mmc_init(int peri, int bus_width)
{
	int index = (peri == PERIPH_ID_SDMMC2) ? 2 : (peri - PERIPH_ID_SDMMC0);
	unsigned int base = ELFIN_HSMMC_BASE + (HSMMC_REG_OFFSET * index);

	return s3c_sdhci_init(base, index, bus_width);
}

#endif /* __ASSEMBLY__ */

#endif  /* __S3C64XX_SDHC_H__ */
