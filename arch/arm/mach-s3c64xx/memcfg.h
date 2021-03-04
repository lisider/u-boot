#ifndef __S3C64XX_MEMCFG_H__
#define __S3C64XX_MEMCFG_H__

#include <asm/arch/clk.h>

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define DMC1_MEM_CFG		0x0001001a	/* Supports one CKE control, Chip1, Burst4, Row/Column bit */

#define DMC1_MEM_CFG2		0xB45
#define DMC1_CHIP0_CFG		0x150F0
#define DMC_DDR_32_CFG		0x0 		/* 32bit, DDR */

/* Memory Parameters */
/* DDR Parameters */
#define DDR_tREFRESH		7800		/* ns */
#define DDR_tRAS		45		/* ns (min: 45ns)*/
#define DDR_tRC 		68		/* ns (min: 67.5ns)*/
#define DDR_tRCD		23		/* ns (min: 22.5ns)*/
#define DDR_tRFC		80		/* ns (min: 80ns)*/
#define DDR_tRP 		23		/* ns (min: 22.5ns)*/
#define DDR_tRRD		15		/* ns (min: 15ns)*/
#define DDR_tWR 		15		/* ns (min: 15ns)*/
#define DDR_tXSR		120		/* ns (min: 120ns)*/
#define DDR_CASL		3		/* CAS Latency 3 */

/*
 * mDDR memory configuration
 */
#define DMC_DDR_BA_EMRS 	2
#define DMC_DDR_MEM_CASLAT	3
#define DMC_DDR_CAS_LATENCY	(DDR_CASL<<1)						/* 6   Set Cas Latency to 3 */
#define DMC_DDR_t_DQSS		1							/* Min 0.75 ~ 1.25 */
#define DMC_DDR_t_MRD		2							/* Min 2 tck */
#define DMC_DDR_t_RAS		(((Startup_HCLK / 1000 * DDR_tRAS) - 1) / 1000000 + 1)	/* 7, Min 45ns */
#define DMC_DDR_t_RC		(((Startup_HCLK / 1000 * DDR_tRC) - 1) / 1000000 + 1) 	/* 10, Min 67.5ns */
#define DMC_DDR_t_RCD		(((Startup_HCLK / 1000 * DDR_tRCD) - 1) / 1000000 + 1) 	/* 4,5(TRM), Min 22.5ns */
#define DMC_DDR_schedule_RCD	((DMC_DDR_t_RCD - 3) << 3)
#define DMC_DDR_t_RFC		(((Startup_HCLK / 1000 * DDR_tRFC) - 1) / 1000000 + 1) 	/* 11,18(TRM) Min 80ns */
#define DMC_DDR_schedule_RFC	((DMC_DDR_t_RFC - 3) << 5)
#define DMC_DDR_t_RP		(((Startup_HCLK / 1000 * DDR_tRP) - 1) / 1000000 + 1) 	/* 4, 5(TRM) Min 22.5ns */
#define DMC_DDR_schedule_RP	((DMC_DDR_t_RP - 3) << 3)
#define DMC_DDR_t_RRD		(((Startup_HCLK / 1000 * DDR_tRRD) - 1) / 1000000 + 1)	/* 3, Min 15ns */
#define DMC_DDR_t_WR		(((Startup_HCLK / 1000 * DDR_tWR) - 1) / 1000000 + 1)	/* Min 15ns */
#define DMC_DDR_t_WTR		2
#define DMC_DDR_t_XP		2							/* 1tck + tIS(1.5ns) */
#define DMC_DDR_t_XSR		(((Startup_HCLK / 1000 * DDR_tXSR) - 1) / 1000000 + 1)	/* 17, Min 120ns */
#define DMC_DDR_t_ESR		DMC_DDR_t_XSR
#define DMC_DDR_REFRESH_PRD	(((Startup_HCLK / 1000 * DDR_tREFRESH) - 1) / 1000000) 	/* TRM 2656 */
#define DMC_DDR_USER_CONFIG	1							/* 2b01 : mDDR */

#endif /* __S3C64XX_MEMCFG_H__ */
