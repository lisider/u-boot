/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_CLK_H_
#define __ASM_ARM_ARCH_CLK_H_

#include <asm/arch/config.h>

#define APLL	0
#define MPLL	1
#define EPLL	2
#define HPLL	3
#define VPLL	4

#define set_pll(mdiv, pdiv, sdiv)	(1<<31 | mdiv<<16 | pdiv<<8 | sdiv)

/***************************
 * Assume Fin : 12MHz 
 ***************************/

/* fixed MPLL Fout 533MHz at Fin 12MHz */
#define MPLL_MDIV	266
#define MPLL_PDIV	3
#define MPLL_SDIV	1

/* fixed EPLL  Fout 96MHz at Fin 12MHz */
#define EPLL_MDIV	24
#define EPLL_PDIV	3
#define EPLL_SDIV	0
#define EPLL_KDIV	0

#if (CFG_CLK_SET == CFG_CLK_A666_M532_E96) /* APLL Fout 666MHz */
#define APLL_MDIV	333
#define APLL_PDIV	3
#define APLL_SDIV	1

#elif (CFG_CLK_SET == CFG_CLK_A532_M532_E96) /* APLL Fout 532MHz */
#define APLL_MDIV	266
#define APLL_PDIV	3
#define APLL_SDIV	1
#define CFG_SYNC_MODE

#elif (CFG_CLK_SET == CFG_CLK_A800_M532_E96) /* APLL Fout 800MHz */
#define APLL_MDIV	400
#define APLL_PDIV	3
#define APLL_SDIV	1

#elif (CFG_CLK_SET == CFG_CLK_A400_M532_E96) /*APLL Fout 400MHz */
#define APLL_MDIV	400
#define APLL_PDIV	3
#define APLL_SDIV	2

#elif (CFG_CLK_SET == CFG_CLK_OTHERS)
/*If you have to use another value, please define pll value here*/
/* FIN 12MHz, APLL Fout 532MHz */
#define APLL_MDIV	266
#define APLL_PDIV	3
#define APLL_SDIV	1
#define CFG_SYNC_MODE
#else
#error "Not Support Fequency or Mode!! you have to setup right configuration."
#endif

#define APLL_VAL    set_pll(APLL_MDIV, APLL_PDIV, APLL_SDIV)
#define MPLL_VAL    set_pll(MPLL_MDIV, MPLL_PDIV, MPLL_SDIV)
#define EPLL_VAL0   set_pll(EPLL_MDIV, EPLL_PDIV, EPLL_SDIV)
#define EPLL_VAL1   (EPLL_KDIV)

/* prevent overflow */
#define Startup_APLL	(CONFIG_SYS_CLK_FREQ/(APLL_PDIV<<APLL_SDIV)*APLL_MDIV)
#define Startup_MPLL	((CONFIG_SYS_CLK_FREQ)/(MPLL_PDIV<<MPLL_SDIV)*MPLL_MDIV)

/*
 * ARM_CLK = APLL / (ARM_RATIO+1)
 * DoutMPLL = MPLL / (MPLL_RATIO+1)
 * HCLKx2_IN is from :  APLL  when sync ; DoutMPLL when async
 * HCLKX2 = HCLKx2_IN / (HCLKX2_RATIO + 1)
 * HCLK = HCLKX2 / (HCLK_RATIO + 1)
 * PCLK = HCLKX2 / (PCLK_RATIO + 1)
*/
#define ARM_RATIO		0
#define MPLL_RATIO		1
#define HCLKX2_RATIO	1
#define HCLK_RATIO		1
#define PCLK_RATIO		3

#if  !defined(CFG_SYNC_MODE)  /* hclkx2 soure from MPLL*/
/* fixed :  HCLKx2 = 266 Mz ,HCLK=133, PCLK=66MH*/
#define CFG_UART_66	 /* default clock value of CLK_UART */

#elif (CFG_CLK_SET == CFG_CLK_A800_M532_E96)
#define HCLKX2_RATIO	3
/*   HCLKx2 = 200 Mz ,HCLK=100, PCLK=50MH*/
#define CFG_UART_50

#elif (CFG_CLK_SET == CFG_CLK_A400_M532_E96)
/*   HCLKx2 = 200 Mz ,HCLK=100, PCLK=50MH*/
#define CFG_UART_50

#else /* CFG_CLK_A532_M532_E96 */
/*   HCLKx2 = 266 Mz ,HCLK=133, PCLK=66MH*/
#define CFG_UART_66
#endif

#define CLK_DIV_VAL	((PCLK_RATIO<<12)|(HCLKX2_RATIO<<9)|(HCLK_RATIO<<8)|(MPLL_RATIO<<4)|ARM_RATIO)

/* calc UART_RATIO, CLKUART = CLKUARTIN / (UART_RATIO + 1) */
#ifdef CFG_CLKSRC_CLKUART  /* source from DoutMPLL= MPLL_Fout /2 = 266MHz */
#undef CFG_UART_50
#define CFG_UART_66	 /* default clock value of CLK_UART */
#endif

#if defined(CFG_SYNC_MODE)
#define Startup_HCLK    (Startup_APLL/(HCLKX2_RATIO+1)/(HCLK_RATIO+1))
#else
#define Startup_HCLK    (Startup_MPLL/(HCLKX2_RATIO+1)/(HCLK_RATIO+1))
#endif

/***********CLK ID  defines*************/
#define CLK_ID_INVALID	 	0
#define CLK_ID_EXTAL		1  /* External Input Clocks */
#define CLK_ID_IIS_CDCLK0	2
#define CLK_ID_IIS_CDCLK1	3
#define CLK_ID_IIS_CDCLK_V4	4
#define CLK_ID_PCM_CDCLK	5
#define CLK_ID_FOUT_APLL	6 /* Internal Core Clocks */
#define CLK_ID_FOUT_MPLL	7
#define CLK_ID_FOUT_EPLL	8
#define CLK_ID_MOUT_APLL	9
#define CLK_ID_MOUT_MPLL	10
#define CLK_ID_MOUT_EPLL	11
#define CLK_ID_DOUT_MPLL	12
#define CLK_ID_ARMCLK		13
#define CLK_ID_HCLK 		14
#define CLK_ID_HCLK2		15
#define CLK_ID_PCLK 		16
#define CLK_ID_27M 		17
#define CLK_ID_48M 		18
#define CLK_ID_PCLK_RTC 	19 /* func Module Clocks */
#define CLK_ID_PCLK_ADC 	20
#define CLK_ID_PCLK_I2C0	21
#define CLK_ID_PCLK_I2C1	22
#define CLK_ID_PCLK_KEYPAD	23
#define CLK_ID_PCLK_SPI0	24
#define CLK_ID_PCLK_SPI1	25
#define CLK_ID_PCLK_AC97	26
#define CLK_ID_PCLK_MFC 	27
#define CLK_ID_PCLK_GPIO	28
#define CLK_ID_PCLK_PWM 	29
#define CLK_ID_PCLK_UART0	30
#define CLK_ID_PCLK_UART1	31
#define CLK_ID_PCLK_UART2	32
#define CLK_ID_PCLK_UART3	33
#define CLK_ID_PCLK_WDT 	34
#define CLK_ID_PCLK_IIS0	35
#define CLK_ID_PCLK_IIS1	36
#define CLK_ID_PCLK_IIS2	37
#define CLK_ID_HCLK_CFCON	38
#define CLK_ID_HCLK_DMA0	39
#define CLK_ID_HCLK_DMA1	40
#define CLK_ID_HCLK_3DSE	41
#define CLK_ID_HCLK_MFC 	42
#define CLK_ID_HCLK_SECUR	43
#define CLK_ID_HCLK_JPEG	44
#define CLK_ID_HCLK_SCALER	45
#define CLK_ID_HCLK_TV		46
#define CLK_ID_HCLK_SDMA0	47
#define CLK_ID_HCLK_SDMA1	48
#define CLK_ID_HCLK_CAMIF	49
#define CLK_ID_HCLK_2D		50
#define CLK_ID_HCLK_POST0	51
#define CLK_ID_HCLK_ROT 	52
#define CLK_ID_HCLK_NAND	53
#define CLK_ID_HCLK_LCD 	54
#define CLK_ID_HCLK_USBHOST	55
#define CLK_ID_HCLK_OTG 	56
#define CLK_ID_HCLK_MMC0	57
#define CLK_ID_HCLK_MMC1	58	
#define CLK_ID_HCLK_MMC2	59
#define CLK_ID_SCLK_MFC 	60  /* special clocks begin*/
#define CLK_ID_SCLK_SECUR	61
#define CLK_ID_SCLK_JPEG	62
#define CLK_ID_SCLK_SCALER	63
#define CLK_ID_SCLK_SCALER27	64
#define CLK_ID_SCLK_CAMIF	65
#define CLK_ID_SCLK_DAC27	66
#define CLK_ID_SCLK_IRDA	67
#define CLK_ID_SCLK_TV27	68
#define CLK_ID_SCLK_USBHOST	69	
#define CLK_ID_SCLK_POST0_27	70
#define CLK_ID_SCLK_MMC0_48M	71
#define CLK_ID_SCLK_MMC1_48M	72
#define CLK_ID_SCLK_MMC2_48M	73
#define CLK_ID_SCLK_MMC0	74
#define CLK_ID_SCLK_MMC1	75
#define CLK_ID_SCLK_MMC2	76
#define CLK_ID_SCLK_UART	77
#define CLK_ID_SCLK_SPI0_48	78
#define CLK_ID_SCLK_SPI1_48	79
#define CLK_ID_SCLK_SPI0	80
#define CLK_ID_SCLK_SPI1	81
#define CLK_ID_SCLK_AUDIO0	82
#define CLK_ID_SCLK_AUDIO1	83
#define CLK_ID_SCLK_AUDIO2	84
#define CLK_ID_SCLK_LCD 	85
#define CLK_ID_SCLK_LCD27	86
#define CLK_ID_LAST_MAX 	87 

#define CLK_ID_MAX_NUMS  (CLK_ID_LAST_MAX)
/***********CLK ID end******************/

#ifndef __ASSEMBLY__

#if CONFIG_IS_ENABLED(DM)
 typedef struct s3c_clk {
	int  id;
	char *name;
	int  usage;
	unsigned long  rate;
	struct s3c_clk_ops *ops;
	struct clksource *sources;
	struct s3c_clk *parent;
}s3c_clk_t;


/**
 * struct clksrc_reg - register definition for clock control bits
 * @reg: pointer to the register in virtual memory.
 * @shift: the shift in bits to where the bitfield is.
 * @size: the size in bits of the bitfield.
 *
 * This specifies the size and position of the bits we are interested
 * in within the register specified by @reg.
 */
struct clksrc_reg {
	void __iomem		*reg;
	unsigned short		shift;
	unsigned short		size;
};

/**
 * struct clksource - list of sources for a given clock
 * @sources: array of pointers to clocks
 * @nr_sources: The size of @sources
 */
struct clksource {
	unsigned int	nr_sources;
	struct s3c_clk	**sources;
	struct clksrc_reg reg_src;
};

int s3c_clk_enable(s3c_clk_t *clk, int enable);
int s3c_clk_set_rate(s3c_clk_t *clk, unsigned long rate);
int s3c_clk_set_parent(s3c_clk_t *clk, s3c_clk_t *parent);
unsigned long s3c_clk_get_rate(s3c_clk_t *clk);
long s3c_clk_round_rate(s3c_clk_t *clk, unsigned long rate);

s3c_clk_t * s3c_clk_get_by_id(int clk_id);

static inline int s3c_clk_enable_by_id(int clk_id, int enable) 
{
	s3c_clk_t *clk = s3c_clk_get_by_id(clk_id);
	return s3c_clk_enable(clk, enable);
}
static inline int s3c_clk_set_rate_by_id(int clk_id, unsigned long rate) 
{
	s3c_clk_t *clk = s3c_clk_get_by_id(clk_id);
	return s3c_clk_set_rate(clk, rate);
}
static inline int s3c_clk_set_parent_by_id(int clk_id, int parent_id) 
{
	s3c_clk_t *clk = s3c_clk_get_by_id(clk_id);
	s3c_clk_t *parent = s3c_clk_get_by_id(parent_id);
	return s3c_clk_set_parent(clk, parent);
}
static inline unsigned long s3c_clk_get_rate_by_id(int clk_id) 
{
	s3c_clk_t *clk = s3c_clk_get_by_id(clk_id);
	return s3c_clk_get_rate(clk);
}
static inline long s3c_clk_round_rate_by_id(int clk_id, unsigned long rate)
{
	s3c_clk_t *clk = s3c_clk_get_by_id(clk_id);
	return s3c_clk_round_rate(clk, rate);
}

#endif  /* CONFIG_IS_ENABLED(DM) */

//unsigned long get_pll_clk(int pllreg);
unsigned long get_arm_clk(void);
unsigned long get_pwm_clk(void);
unsigned long get_uart_clk(int dev_index);

#endif  /* __ASSEMBLY__ */

#endif
