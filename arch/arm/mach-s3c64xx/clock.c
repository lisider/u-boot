/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */
#include <div64.h>

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <linux/err.h>

#include <asm/arch/cpu.h>
#include <asm/arch/clock-regs.h>
#include <asm/arch/clk.h>
#include <asm/arch/config.h>

//#define pr_debug pr_err  /* for debug */

#if CONFIG_IS_ENABLED(DM)

/**
 * struct s3c_clk_ops - standard clock operations
 * @enable: enable/disable clock, see clk_enable() and clk_disable()
 * @set_rate: set the clock rate, see clk_set_rate().
 * @get_rate: get the clock rate, see clk_get_rate().
 * @round_rate: round a given clock rate, see clk_round_rate().
 * @set_parent: set the clock's parent, see clk_set_parent().
 *
 * Group the common clock implementations together so that we
 * don't have to keep setting the same fiels again. We leave
 * enable in s3c_clk_t .
 *
 */
struct s3c_clk_ops {
	int (*enable) (struct s3c_clk *c, int enable);
	int (*set_rate) (struct s3c_clk *c, unsigned long rate);
	unsigned long (*get_rate) (struct s3c_clk *c);
	unsigned long (*round_rate) (struct s3c_clk *c, unsigned long rate);
	int (*set_parent) (struct s3c_clk *c, struct s3c_clk *parent);
};

typedef struct s3c64xx_clksrc {
	s3c_clk_t clk;
	unsigned long   ctrlbit;
	int (*enable)(s3c_clk_t *c, int enable);
	struct clksrc_reg	reg_div;  /* divide the source clk */
}s3c64xx_clksrc_t;

//#define  USING_CLKSRC_LINK
#ifdef USING_CLKSRC_LINK

struct clksrc_link {
	struct s3c_clk *clk;
	struct clksrc_link *next;
};

static struct clksrc_link *s3c64xx_clklist_link_head = NULL;

int  s3c_clklist_link_add(s3c_clk_t *clk)
{
	struct clksrc_link *p,*ptr=s3c64xx_clklist_link_head;

	while(ptr->clk && ptr->next) ptr=ptr->next;

	if(NULL == ptr->clk)
		p=ptr;
	else {
		p = calloc(1,sizeof(struct clksrc_link));
		if(!p) {
			pr_err("%s,malloc failed\n",__func__);
			return -ENOMEM;
		}
		ptr->next=p;
	}

	p->clk = clk;

	pr_debug("clksrc: add new link item clock \"%s\"\n",p->clk->name);
	return 0;	
}

s3c_clk_t * s3c_clk_get_by_id(int clk_id)
{
	struct clksrc_link *ptr=s3c64xx_clklist_link_head;

	while(ptr) {
		 if(ptr->clk->id == clk_id)
		 	return ptr->clk;
		ptr = ptr->next;
	}

	return NULL;
}

#else /* ! USING_CLKSRC_LINK */
struct clksrc_array {
	struct s3c_clk *clklist[CLK_ID_MAX_NUMS];
};

static struct clksrc_array *s3c64xx_clklist_head = NULL;

int  s3c_clklist_link_add(s3c_clk_t *clk)
{
	struct clksrc_array *link = s3c64xx_clklist_head;
	if(clk->id < CLK_ID_MAX_NUMS) {
		link->clklist[clk->id] = clk;
	}
	
	return 0;
}
s3c_clk_t * s3c_clk_get_by_id(int clk_id)
{
	struct clksrc_array *link = s3c64xx_clklist_head;
	if(clk_id < CLK_ID_MAX_NUMS) {
		return link->clklist[clk_id];
	}
	
	return 0;
}
#endif /* USING_CLKSRC_LINK */

static inline u32 bit_mask(u32 shift, u32 nr_bits)
{
	u32 mask = 0xffffffff >> (32 - nr_bits);

	return mask << shift;
}

static inline struct s3c64xx_clksrc *to_clksrc(s3c_clk_t *clk)
{
	return container_of(clk, struct s3c64xx_clksrc, clk);
}


#define S3C64XX_PLL_MDIV_MASK		(0x3FF)
#define S3C64XX_PLL_PDIV_MASK		(0x3F)
#define S3C64XX_PLL_SDIV_MASK		(0x7)
#define S3C64XX_PLL_MDIV_SHIFT		(16)
#define S3C64XX_PLL_PDIV_SHIFT		(8)
#define S3C64XX_PLL_SDIV_SHIFT		(0)

static inline unsigned long s3c_get_pll(unsigned long baseclk,
					    u32 pllcon)
{
	u32 mdiv, pdiv, sdiv;
	u64 fvco = baseclk;

	mdiv = (pllcon >> S3C64XX_PLL_MDIV_SHIFT) & S3C64XX_PLL_MDIV_MASK;
	pdiv = (pllcon >> S3C64XX_PLL_PDIV_SHIFT) & S3C64XX_PLL_PDIV_MASK;
	sdiv = (pllcon >> S3C64XX_PLL_SDIV_SHIFT) & S3C64XX_PLL_SDIV_MASK;

	fvco *= mdiv;
	do_div(fvco, (pdiv << sdiv));

	return (unsigned long)fvco;
}

#define PLL6553X_MDIV_MASK	(0x7F)
#define PLL6553X_PDIV_MASK	(0x1F)
#define PLL6553X_SDIV_MASK	(0x3)
#define PLL6553X_KDIV_MASK	(0xFFFF)
#define PLL6553X_MDIV_SHIFT	(16)
#define PLL6553X_PDIV_SHIFT	(8)
#define PLL6553X_SDIV_SHIFT	(0)

static inline unsigned long s3c_get_pll6553x(unsigned long baseclk,
					     u32 pll_con0, u32 pll_con1)
{
	unsigned long result;
	u32 mdiv, pdiv, sdiv, kdiv;
	u64 tmp;

	mdiv = (pll_con0 >> PLL6553X_MDIV_SHIFT) & PLL6553X_MDIV_MASK;
	pdiv = (pll_con0 >> PLL6553X_PDIV_SHIFT) & PLL6553X_PDIV_MASK;
	sdiv = (pll_con0 >> PLL6553X_SDIV_SHIFT) & PLL6553X_SDIV_MASK;
	kdiv = pll_con1 & PLL6553X_KDIV_MASK;

	/*
	 * We need to multiple baseclk by mdiv (the integer part) and kdiv
	 * which is in 2^16ths, so shift mdiv up (does not overflow) and
	 * add kdiv before multiplying. The use of tmp is to avoid any
	 * overflows before shifting bac down into result when multipling
	 * by the mdiv and kdiv pair.
	 */

	tmp = baseclk;
	tmp *= (mdiv << 16) + kdiv;
	do_div(tmp, (pdiv << sdiv));
	result = tmp >> 16;

	return result;
}

int s3c_clk_enable(s3c_clk_t *clk, int enable)
{
	if (IS_ERR(clk) || clk == NULL)
		return -EINVAL;

	if (clk->parent != NULL)
		s3c_clk_enable(clk->parent, enable);

	if(clk->ops != NULL && clk->ops->enable != NULL) {
		if (enable && (clk->usage++) == 0)
			return (clk->ops->enable)(clk, 1);
		if (!enable && (--clk->usage) == 0)
			return (clk->ops->enable)(clk, 0);
	}

	return 0;
}

unsigned long s3c_clk_get_rate(s3c_clk_t *clk)
{
	if (IS_ERR_OR_NULL(clk))
		return 0;

	if (clk->rate != 0)
		return clk->rate;

	if (clk->ops != NULL && clk->ops->get_rate != NULL)
		return (clk->ops->get_rate)(clk);

	if (clk->parent != NULL)
		return s3c_clk_get_rate(clk->parent);

	return clk->rate;
}

long s3c_clk_round_rate(s3c_clk_t *clk, unsigned long rate)
{
	if (!IS_ERR_OR_NULL(clk) && clk->ops && clk->ops->round_rate)
		return (clk->ops->round_rate)(clk, rate);

	return rate;
}

int s3c_clk_set_rate(s3c_clk_t *clk, unsigned long rate)
{
	int ret;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	/* We do not default just do a clk->rate = rate as
	 * the clock may have been made this way by choice.
	 */

	//WARN_ON(clk->ops == NULL);
	//WARN_ON(clk->ops && clk->ops->set_rate == NULL);

	if (clk->ops == NULL || clk->ops->set_rate == NULL)
		return -EINVAL;

	ret = (clk->ops->set_rate)(clk, rate);

	return ret;
}

int s3c_clk_set_parent(s3c_clk_t *clk, s3c_clk_t *parent)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(clk) || IS_ERR_OR_NULL(parent))
		return -EINVAL;

	if (clk->ops && clk->ops->set_parent)
		ret = (clk->ops->set_parent)(clk, parent);

	return ret;
}

/***************************************************/
static unsigned long s3c64xx_clk_arm_get_rate(s3c_clk_t *clk)
{
	unsigned long rate = s3c_clk_get_rate(clk->parent);
	u32 clkdiv;

	/* divisor mask starts at bit0, so no need to shift */
	clkdiv = __raw_readl(S3C_CLK_DIV0) & S3C6410_CLKDIV0_ARM_MASK;

	return rate / (clkdiv + 1);
}

static unsigned long s3c64xx_clk_arm_round_rate(s3c_clk_t *clk,
						unsigned long rate)
{
	unsigned long parent = s3c_clk_get_rate(clk->parent);
	u32 div;

	if (parent < rate)
		return parent;

	div = (parent / rate) - 1;
	if (div > S3C6410_CLKDIV0_ARM_MASK)
		div = S3C6410_CLKDIV0_ARM_MASK;

	return parent / (div + 1);
}

static int s3c64xx_clk_arm_set_rate(s3c_clk_t *clk, unsigned long rate)
{
	unsigned long parent = s3c_clk_get_rate(clk->parent);
	u32 div;
	u32 val;

	if (rate < parent / (S3C6410_CLKDIV0_ARM_MASK + 1))
		return -EINVAL;

	rate = s3c_clk_round_rate(clk, rate);
	div = s3c_clk_get_rate(clk->parent) / rate;

	val = __raw_readl(S3C_CLK_DIV0);
	val &= ~S3C6410_CLKDIV0_ARM_MASK;
	val |= (div - 1);
	__raw_writel(val, S3C_CLK_DIV0);

	return 0;

}

struct s3c_clk_ops	clk_arm_ops = {
	.get_rate 	= s3c64xx_clk_arm_get_rate,
	.set_rate 	= s3c64xx_clk_arm_set_rate,
	.round_rate	= s3c64xx_clk_arm_round_rate,
};

#define clk_fin_apll clk_xtal
#define clk_fin_mpll clk_xtal
#define clk_fin_epll clk_xtal

s3c_clk_t clk_xtal = {
	.id		= CLK_ID_EXTAL,
	.name	= "xtal",
	.rate		= CONFIG_SYS_CLK_FREQ,
};

static s3c_clk_t clk_fout_apll = {
	.id		= CLK_ID_FOUT_APLL,
	.name	= "fout-apll",
};

static s3c_clk_t clk_fout_mpll = {
	.id		= CLK_ID_FOUT_MPLL,
	.name	= "fout-mpll",
};


static s3c_clk_t clk_fout_epll = {
	.id		= CLK_ID_FOUT_EPLL,
	.name	= "fout-epll",
};

static s3c_clk_t *clk_src_apll_list[] = {
	[0] = &clk_fin_apll,
	[1] = &clk_fout_apll,
};

static struct clksource clk_src_apll = {
	.sources	= clk_src_apll_list,
	.nr_sources	= ARRAY_SIZE(clk_src_apll_list),
	.reg_src	= { .reg = (void*)S3C_CLK_SRC, .shift = 0, .size = 1  },
};

s3c_clk_t clk_mout_apll = {
	.id		= CLK_ID_MOUT_APLL,
	.name	= "mout-apll",
	.sources	= &clk_src_apll,
};

static s3c_clk_t *clk_src_mpll_list[] = {
	[0] = &clk_fin_mpll,
	[1] = &clk_fout_mpll,
};

static struct clksource clk_src_mpll = {
	.sources	= clk_src_mpll_list,
	.nr_sources	= ARRAY_SIZE(clk_src_mpll_list),
	.reg_src	= { .reg = (void*)S3C_CLK_SRC, .shift = 1, .size = 1  },
};

s3c_clk_t clk_mout_mpll = {
	.id		= CLK_ID_MOUT_MPLL,
	.name	= "mout-mpll",
	.sources	= &clk_src_mpll,
};

static s3c_clk_t *clk_src_epll_list[] = {
	[0] = &clk_fin_epll,
	[1] = &clk_fout_epll,
};

static struct clksource clk_src_epll = {
	.sources	= clk_src_epll_list,
	.nr_sources	= ARRAY_SIZE(clk_src_epll_list),
	.reg_src	= { .reg = (void*)S3C_CLK_SRC, .shift = 2, .size = 1  },
};

s3c_clk_t clk_mout_epll = {
	.id		= CLK_ID_MOUT_EPLL,
	.name	= "mout-epll",
	.sources	= &clk_src_epll,
};

s3c_clk_t clk_h2 = {
	.id		= CLK_ID_HCLK2,
	.name	= "hclk2",
	.rate		= 0,       /* calc in s3c64xx_setup_clocks()*/
	.parent	= NULL,  /* init in s3c64xx_setup_clocks()*/
};

s3c_clk_t clk_h = {
	.id		= CLK_ID_HCLK,
	.name	= "hclk",
	.rate		= 0,       /* calc in s3c64xx_setup_clocks()*/
	.parent	= &clk_h2,
 };

s3c_clk_t clk_p = {
	.id		= CLK_ID_PCLK,
	.name	= "pclk",
	.rate		= 0,       /* calc in s3c64xx_setup_clocks()*/
	.parent	= &clk_h2,
};

static s3c_clk_t clk_arm = {   /* clock for ARM1176 */
	.id		= CLK_ID_ARMCLK,
	.name	= "armclk",
	.parent	= &clk_mout_apll,
	.ops		= &clk_arm_ops,
};

static unsigned long s3c64xx_clk_doutmpll_get_rate(s3c_clk_t *clk)
{
	unsigned long rate = s3c_clk_get_rate(clk->parent);

	pr_debug( "%s: parent is %ld\n", __func__, rate);

	if (__raw_readl(S3C_CLK_DIV0) & S3C64XX_CLKDIV0_MPLL_MASK)
		rate /= 2;

	return rate;
}

static s3c_clk_t clk_dout_mpll = {
	.id		= CLK_ID_DOUT_MPLL,
	.name	= "dout-mpll",
	.parent	= &clk_mout_mpll,
	.ops		= &(struct s3c_clk_ops) {
		.get_rate	= s3c64xx_clk_doutmpll_get_rate,
	},
};

s3c_clk_t clk_27m = {
	.id		= CLK_ID_27M,
	.name	= "clk27m",
	.rate		= 27000000,
};

static int clk_48m_ctrl(s3c_clk_t *clk, int enable)
{
	u32 val;

	val = __raw_readl(S3C_OTHERS);
	if (enable)
		val |= S3C64XX_OTHERS_USBMASK;
	else
		val &= ~S3C64XX_OTHERS_USBMASK;

	__raw_writel(val, S3C_OTHERS);

	return 0;
}

s3c_clk_t clk_48m = {
	.id		= CLK_ID_48M,
	.name	= "clk48m",
	.rate 	= 48000000,
	.ops 	= &(struct s3c_clk_ops) {
		.enable	= clk_48m_ctrl,
	}
};

static s3c_clk_t *listptr_clk_parents[] = {
	&clk_mout_apll,
	&clk_mout_epll,
	&clk_mout_mpll,
	&clk_fin_epll,
	&clk_h2,
	&clk_h,
	&clk_p,
	&clk_arm,
	&clk_dout_mpll,
	&clk_27m,
	&clk_48m
};

static s3c_clk_t *clkset_mfc_list[] = {
	&clk_h2,
	&clk_mout_epll,
};

static struct clksource clkset_mfc = {
	.sources		= clkset_mfc_list,
	.nr_sources	= ARRAY_SIZE(clkset_mfc_list),
	.reg_src		= { .reg = (void*)S3C_CLK_SRC, .shift = 4, .size = 1  },
};

static s3c_clk_t *clkset_scaler_list[] = {
	&clk_mout_epll,
	&clk_dout_mpll,
	&clk_fin_epll,
	NULL
};

static struct clksource clkset_scaler = {
	.sources		= clkset_scaler_list,
	.nr_sources	= ARRAY_SIZE(clkset_scaler_list),
	.reg_src		= { .reg = (void*)S3C_CLK_SRC, .shift = 28, .size = 2  },
};

static int inline s3c64xx_gate(u32 *reg,
				s3c_clk_t *clk,
				int enable)
{
	struct s3c64xx_clksrc *sclk = to_clksrc(clk);
	unsigned int ctrlbit = sclk->ctrlbit;
	u32 con;

	pr_debug("%s,clock \"%s\" -- %s\n",__func__,clk->name,
					enable? "enable":"diable");
	con = __raw_readl(reg);

	if (enable)
		con |= ctrlbit;
	else
		con &= ~ctrlbit;

	__raw_writel(con, reg);
	return 0;
}

static int s3c64xx_pclk_ctrl(s3c_clk_t *clk, int enable)
{
	return s3c64xx_gate((u32*)S3C_PCLK_GATE, clk, enable);
}

static int s3c64xx_hclk_ctrl(s3c_clk_t *clk, int enable)
{
	return s3c64xx_gate((u32*)S3C_HCLK_GATE, clk, enable);
}

int s3c64xx_sclk_ctrl(s3c_clk_t *clk, int enable)
{
	return s3c64xx_gate((u32*)S3C_SCLK_GATE, clk, enable);
}

static struct s3c64xx_clksrc list_clksrc_init_off[] = {
	{
		.clk = {
			.id		= CLK_ID_PCLK_RTC,
			.name	= "pclk-rtc",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_RTC,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_ADC,
			.name	= "pclk-adc",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_TSADC,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_I2C0,
			.name	= "pclk-i2c0",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_IIC,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_I2C1,
			.name	= "pclk-i2c1",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C6410_CLKCON_PCLK_I2C1,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_KEYPAD,
			.name	= "pclk-i2c1",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_KEYPAD,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_SPI0,
			.name	= "pclk-spi0",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_SPI0,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_SPI1,
			.name	= "pclk-spi1",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_SPI1,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_AC97,
			.name	= "pclk-ac97",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_AC97,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_MFC,
			.name	= "pclk-mfc",
			.parent	= &clk_p,
		},
		.enable 	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_MFC,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_CFCON,
			.name	= "hclk-cfcon",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_IHOST,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_DMA0,
			.name	= "hclk-dma0",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_DMA0,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_DMA1,
			.name	= "hclk-dma1",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_DMA1,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_3DSE,
			.name	= "hclk-3dse",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_3DSE,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_SECUR,
			.name	= "hclk-secur",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_SECUR,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_SDMA0,
			.name	= "hclk-sdma0",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_SDMA0,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_SDMA1,
			.name	= "hclk-sdma1",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_SDMA1,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_JPEG,
			.name	= "hclk-jpeg",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_JPEG,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_CAMIF,
			.name	= "hclk-camif",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_CAMIF,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_SCALER,
			.name	= "hclk-scaler",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_SCALER,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_2D,
			.name	= "hclk-2d",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_2D,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_TV,
			.name	= "hclk-tv",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_TV,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_POST0,
			.name	= "hclk-post0",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_POST0,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_ROT,
			.name	= "hclk-rot",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_ROT,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_MFC,
			.name	= "hclk-mfc",
			.parent	= &clk_h,
		},
		.enable 	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_MFC,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_MMC0_48M,
			.name	= "sclk-mmc0-48m",
			.parent	= &clk_48m,
		},
		.enable 	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_MMC0_48,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_MMC1_48M,
			.name	= "sclk-mmc1-48m",
			.parent	= &clk_48m,
		},
		.enable 	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_MMC1_48,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_MMC2_48M,
			.name	= "sclk-mmc2-48m",
			.parent	= &clk_48m,
		},
		.enable 		= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_MMC2_48,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_DAC27,
			.name	= "sclk-dac-27m",
			.parent	= &clk_27m,
		},
		.enable 	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_DAC27,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_TV27,
			.name	= "sclk-tv-27m",
			.parent	= &clk_27m,
		},
		.enable 	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_TV27,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_SCALER27,
			.name	= "sclk-scaler-27m",
			.parent	= &clk_27m,
		},
		.enable 	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_SCALER27,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_SCALER,
			.name	= "sclk-scaler",
			.sources	= &clkset_scaler,
		},
		.enable 	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_SCALER,
		.reg_div	= { .reg = (void*)S3C_CLK_DIV1, .shift = 16, .size = 4  },
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_POST0_27,
			.name	= "sclk-post0-27m",
			.parent	= &clk_27m,
		},
		.enable 	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_POST0_27,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_LCD27,
			.name	= "sclk-lcd-27m",
			.parent	= &clk_27m,
		},
		.enable 	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_LCD27,
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_SECUR,
			.name	= "sclk-secur",
			.parent	= &clk_h2,
		},
		.enable  	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_SECUR,
		.reg_div	= { .reg = (void*)S3C_CLK_DIV0, .shift = 18, .size = 2  },
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_MFC,
			.name	= "sclk-mfc",
			.sources	= &clkset_mfc,
		},
		.enable  		= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_MFC,
		.reg_div	= { .reg = (void*)S3C_CLK_DIV0, .shift = 28, .size = 4  },
	}, {
		.clk = {
			.id		= CLK_ID_SCLK_JPEG,
			.name	= "sclk-jpeg",
			.parent	= &clk_h2,
		},
		.enable  	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_JPEG,
		.reg_div	= { .reg = (void*)S3C_CLK_DIV0, .shift = 24, .size = 4  },
	},
};

static s3c_clk_t *clkset_uart_list[] = {
	&clk_mout_epll,
	&clk_dout_mpll,
	NULL,
	NULL
};

static struct clksource clkset_uart = {
	.sources	= clkset_uart_list,
	.nr_sources	= ARRAY_SIZE(clkset_uart_list),
	.reg_src    = { .reg = (void*)S3C_CLK_SRC, .shift = 13, .size = 1  },
};

static struct s3c64xx_clksrc list_clksrc_init_on[] = {
	{
		.clk = {
			.id		= CLK_ID_HCLK_LCD,
			.name	= "hclk-lcd",
			.parent	= &clk_h,
		},
		.enable  	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_LCD,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_GPIO,
			.name	= "pclk-gpio",
			.parent	= &clk_p,
		},
		.enable  	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_GPIO,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_USBHOST,
			.name	= "hclk-usbhost",
			.parent	= &clk_h,
		},
		.enable  	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_UHOST,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_OTG,
			.name	= "hclk-otg",
			.parent	= &clk_h,
		},
		.enable  	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_USB,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_PWM,
			.name	= "pclk-pwm",
			.parent	= &clk_p,
		},
		.enable  	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_PWM,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_UART0,
			.name	= "pclk-uart0",
			.parent	= &clk_p,
		},
		.enable  	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_UART0,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_UART1,
			.name	= "pclk-uart1",
			.parent	= &clk_p,
		},
		.enable  	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_UART1,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_UART2,
			.name	= "pclk-uart2",
			.parent	= &clk_p,
		},
		.enable  	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_UART2,
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_UART3,
			.name	= "pclk-uart3",
			.parent	= &clk_p,
		},
		.enable  	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_UART3,
	}, {
		.clk    = {
			.id       	= CLK_ID_SCLK_UART,
			.name	= "sclk-uart",
			.sources	= &clkset_uart,
		},
		.enable  	= s3c64xx_sclk_ctrl,
		.ctrlbit	= S3C_CLKCON_SCLK_UART,
		.reg_div	= { .reg = (void*)S3C_CLK_DIV2, .shift = 16, .size = 4  },
	}, {
		.clk = {
			.id		= CLK_ID_PCLK_WDT,
			.name	= "pclk-wdt",
			.parent	= &clk_p,
		},
		.enable  	= s3c64xx_pclk_ctrl,
		.ctrlbit	= S3C_CLKCON_PCLK_WDT,
	}, {
		.clk = {
			.id		= CLK_ID_HCLK_NAND,
			.name	= "hclk-nand",
			.parent	= &clk_h,
		},
		.enable  	= s3c64xx_hclk_ctrl,
		.ctrlbit	= S3C_CLKCON_HCLK_MEM0,
	},

};

static struct s3c64xx_clksrc clk_48m_spi0 = {
	.clk = {
		.id		= CLK_ID_SCLK_SPI0_48,
		.name	= "sclk-spi0-48m",
		.parent	= &clk_48m,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_SPI0_48,
};

static struct s3c64xx_clksrc clk_48m_spi1 = {
	.clk = {
		.id		= CLK_ID_SCLK_SPI1_48,
		.name	= "sclk-spi1-48m",
		.parent	= &clk_48m,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_SPI1_48,
};

static struct s3c64xx_clksrc clk_i2s0 = {
	.clk = {
		.id		= CLK_ID_PCLK_IIS0,
		.name	= "pclk-iis0",
		.parent	= &clk_p,
	},
	.enable  	= s3c64xx_pclk_ctrl,
	.ctrlbit	= S3C_CLKCON_PCLK_IIS0,
};

static struct s3c64xx_clksrc clk_i2s1 = {
	.clk = {
		.id		= CLK_ID_PCLK_IIS1,
		.name	= "pclk-iis1",
		.parent	= &clk_p,
	},
	.enable  	= s3c64xx_pclk_ctrl,
	.ctrlbit	= S3C_CLKCON_PCLK_IIS1,
};

static struct s3c64xx_clksrc clk_i2s2 = {
	.clk = {
		.id		= CLK_ID_PCLK_IIS2,
		.name	= "pclk-iis2",
		.parent	= &clk_p,
	},
	.enable  	= s3c64xx_pclk_ctrl,
	.ctrlbit	= S3C6410_CLKCON_PCLK_IIS2,
};

static struct s3c64xx_clksrc clk_hclk_hsmmc0 = {
	.clk = {
		.id		= CLK_ID_HCLK_MMC0,
		.name	= "hclk-mmc0",
		.parent	= &clk_h,
	},
	.enable  	= s3c64xx_hclk_ctrl,
	.ctrlbit	= S3C_CLKCON_HCLK_HSMMC0,
};

static struct s3c64xx_clksrc clk_hclk_hsmmc1 = {
	.clk = {
		.id		= CLK_ID_HCLK_MMC1,
		.name	= "hclk-mmc1",
		.parent	= &clk_h,
	},
	.enable  	= s3c64xx_hclk_ctrl,
	.ctrlbit	= S3C_CLKCON_HCLK_HSMMC1,
};

static struct s3c64xx_clksrc clk_hclk_hsmmc2 = {
	.clk = {
		.id		= CLK_ID_HCLK_MMC2,
		.name	= "hclk-mmc2",
		.parent	= &clk_h,
	},
	.enable  	= s3c64xx_hclk_ctrl,
	.ctrlbit	= S3C_CLKCON_HCLK_HSMMC2,
};

static struct s3c64xx_clksrc *listptr_clksrc_others_singlesrc[] = {
	&clk_48m_spi0,
	&clk_48m_spi1,
	&clk_i2s0,
	&clk_i2s1,
	&clk_i2s2,
	&clk_hclk_hsmmc0,
	&clk_hclk_hsmmc1,
	&clk_hclk_hsmmc2,
};

static s3c_clk_t *clkset_spi_mmc_list[] = {
	&clk_mout_epll,
	&clk_dout_mpll,
	&clk_fin_epll,
	&clk_27m,
};

static struct clksource clkset_mmc0 = {
	.sources		= clkset_spi_mmc_list,
	.nr_sources	= ARRAY_SIZE(clkset_spi_mmc_list),
	.reg_src    	= { .reg = (void*)S3C_CLK_SRC, .shift = 18, .size = 2  },
};

static struct clksource clkset_mmc1 = {
	.sources		= clkset_spi_mmc_list,
	.nr_sources	= ARRAY_SIZE(clkset_spi_mmc_list),
	.reg_src		= { .reg = (void*)S3C_CLK_SRC, .shift = 20, .size = 2  },
};

static struct clksource clkset_mmc2 = {
	.sources		= clkset_spi_mmc_list,
	.nr_sources	= ARRAY_SIZE(clkset_spi_mmc_list),
	.reg_src		= { .reg = (void*)S3C_CLK_SRC, .shift = 22, .size = 2  },
};

static struct s3c64xx_clksrc clk_sclk_mmc0 = {
	.clk    = {
		.id		= CLK_ID_SCLK_MMC0,
		.name	= "sclk-mmc0",
		.sources	= &clkset_mmc0,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_MMC0,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV1, .shift = 0, .size = 4  },
};

static struct s3c64xx_clksrc clk_sclk_mmc1 = {
	.clk    = {
		.id		= CLK_ID_SCLK_MMC1,
		.name	= "sclk-mmc1",
		.sources	= &clkset_mmc1,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_MMC1,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV1, .shift = 4, .size = 4  },
};

static struct s3c64xx_clksrc clk_sclk_mmc2 = {
	.clk    = {
		.id		= CLK_ID_SCLK_MMC2,
		.name	= "sclk-mmc2",
		.sources	= &clkset_mmc2,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_MMC2,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV1, .shift = 8, .size = 4  },
};

static struct clksource clkset_spi0 = {
	.sources	= clkset_spi_mmc_list,
	.nr_sources	= ARRAY_SIZE(clkset_spi_mmc_list),
	.reg_src = { .reg = (void*)S3C_CLK_SRC, .shift = 14, .size = 2 },
};

static struct clksource clkset_spi1 = {
	.sources	= clkset_spi_mmc_list,
	.nr_sources	= ARRAY_SIZE(clkset_spi_mmc_list),
	.reg_src = { .reg = (void*)S3C_CLK_SRC, .shift = 16, .size = 2 },
};

static struct s3c64xx_clksrc clk_sclk_spi0 = {
	.clk	= {
		.id		= CLK_ID_SCLK_SPI0,
		.name	= "sclk-spi0",
		.sources	= &clkset_spi0,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_SPI0,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV2, .shift = 0, .size = 4 },
};

static struct s3c64xx_clksrc clk_sclk_spi1 = {
	.clk	= {
		.id		= CLK_ID_SCLK_SPI1,
		.name	= "sclk-spi1",
		.sources	= &clkset_spi1,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_SPI1,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV2, .shift = 4, .size = 4 },
};


/* The peripheral clocks are all controlled via clocksource followed
 * by an optional divider and gate stage. We currently roll this into
 * one clock which hides the intermediate clock from the mux.
 *
 * Note, the JPEG clock can only be an even divider...
 *
 * The scaler and LCD clocks depend on the S3C64XX version, and also
 * have a common parent divisor so are not included here.
 */

/* clocks that feed other parts of the clock source tree */

static s3c_clk_t clk_iis_cd0 = {
	.id		= CLK_ID_IIS_CDCLK0,
	.name	= "iis-cdclk0",
};

static s3c_clk_t clk_iis_cd1 = {
	.id		= CLK_ID_IIS_CDCLK1,
	.name	= "iis-cdclk1",
};

static s3c_clk_t clk_iisv4_cd = {
	.id		= CLK_ID_IIS_CDCLK_V4,
	.name	= "iis-cdclkv4",
};

static s3c_clk_t clk_pcm_cd = {
	.id		= CLK_ID_PCM_CDCLK,
	.name	= "pcm-cdclk",
};

static s3c_clk_t *clkset_audio0_list[] = {
	[0] = &clk_mout_epll,
	[1] = &clk_dout_mpll,
	[2] = &clk_fin_epll,
	[3] = &clk_iis_cd0,
	[4] = &clk_pcm_cd,
};

static s3c_clk_t *clkset_audio1_list[] = {
	[0] = &clk_mout_epll,
	[1] = &clk_dout_mpll,
	[2] = &clk_fin_epll,
	[3] = &clk_iis_cd1,
	[4] = &clk_pcm_cd,
};

static s3c_clk_t *clkset_audio2_list[] = {
	[0] = &clk_mout_epll,
	[1] = &clk_dout_mpll,
	[2] = &clk_fin_epll,
	[3] = &clk_iisv4_cd,
	[4] = &clk_pcm_cd,
};

static struct clksource clkset_audio0 = {
	.sources		= clkset_audio0_list,
	.nr_sources	= ARRAY_SIZE(clkset_audio0_list),
	.reg_src		= { .reg = (void*)S3C_CLK_SRC, .shift = 7, .size = 3  },
};

static struct clksource clkset_audio1 = {
	.sources		= clkset_audio1_list,
	.nr_sources	= ARRAY_SIZE(clkset_audio1_list),
	.reg_src		= { .reg = (void*)S3C_CLK_SRC, .shift = 10, .size = 3  },
};

static struct clksource clkset_audio2 = {
	.sources		= clkset_audio2_list,
	.nr_sources	= ARRAY_SIZE(clkset_audio2_list),
	.reg_src		= { .reg = (void*)S3C6410_CLK_SRC2, .shift = 0, .size = 3  },
};

static struct s3c64xx_clksrc clk_sclk_audio0 = {
	.clk	= {
		.id		= CLK_ID_SCLK_AUDIO0,
		.name	= "sclk-audio0",
		.sources	= &clkset_audio0,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_AUDIO0,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV2, .shift = 8, .size = 4  },
};

static struct s3c64xx_clksrc clk_sclk_audio1 = {
	.clk	= {
		.id		= CLK_ID_SCLK_AUDIO1,
		.name	= "sclk-audio1",
		.sources	= &clkset_audio1,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_AUDIO1,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV2, .shift = 12, .size = 4  },
};

static struct s3c64xx_clksrc clk_sclk_audio2 = {
	.clk	= {
		.id		= CLK_ID_SCLK_AUDIO2,
		.name	= "sclk-audio2",
		.sources	= &clkset_audio2,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C6410_CLKCON_SCLK_AUDIO2,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV2, .shift = 24, .size = 4  },
};

static s3c_clk_t *clkset_irda_list[] = {
	&clk_mout_epll,
	&clk_dout_mpll,
	&clk_fin_epll,
	&clk_48m,
};

static struct clksource clkset_irda = {
	.sources		= clkset_irda_list,
	.nr_sources	= ARRAY_SIZE(clkset_irda_list),
	.reg_src		= { .reg = (void*)S3C_CLK_SRC, .shift = 24, .size = 2  },
};

static struct s3c64xx_clksrc clk_sclk_irda= {
	.clk	= {
		.id		= CLK_ID_SCLK_IRDA,
		.name	= "sclk-irda",
		.sources	= &clkset_irda,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_IRDA,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV2, .shift = 20, .size = 4  },
};

static s3c_clk_t *clkset_lcd_list[] = {
	&clk_mout_epll,
	&clk_dout_mpll,
	&clk_fin_epll,
	NULL,
};

static struct clksource clkset_lcd = {
	.sources		= clkset_lcd_list,
	.nr_sources	= ARRAY_SIZE(clkset_lcd_list),
	.reg_src		= { .reg = (void*)S3C_CLK_SRC, .shift = 26, .size = 2  },
};

static struct s3c64xx_clksrc clk_sclk_lcd= {
	.clk	= {
		.id		= CLK_ID_SCLK_LCD,
		.name	= "sclk-lcd",
		.sources	= &clkset_lcd,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_LCD,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV1, .shift = 12, .size = 4  },
};

static s3c_clk_t *clkset_uhost_list[] = {
	&clk_48m,
	&clk_mout_epll,
	&clk_dout_mpll,
	&clk_fin_epll,
};

static struct clksource clkset_uhost = {
	.sources		= clkset_uhost_list,
	.nr_sources	= ARRAY_SIZE(clkset_uhost_list),
	.reg_src 		= { .reg = (void*)S3C_CLK_SRC, .shift = 5, .size = 2  },
};

static struct s3c64xx_clksrc clk_sclk_uhost = {
	.clk	= {
		.id		= CLK_ID_SCLK_USBHOST,
		.name	= "sclk-usbhost",
		.sources	= &clkset_uhost,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_UHOST,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV1, .shift = 20, .size = 4  },
};

static struct s3c64xx_clksrc clk_sclk_camif = {
	.clk	= {
		.id		= CLK_ID_SCLK_CAMIF,
		.name	= "sclk-camif",
		.parent	= &clk_h2,
	},
	.enable  	= s3c64xx_sclk_ctrl,
	.ctrlbit	= S3C_CLKCON_SCLK_CAM,
	.reg_div	= { .reg = (void*)S3C_CLK_DIV0, .shift = 20, .size = 4  },
};

static struct s3c64xx_clksrc *listptr_clksrc_others_mulsrc[] = {
	&clk_sclk_audio0,
	&clk_sclk_audio1,
	&clk_sclk_audio2,
	&clk_sclk_spi0,
	&clk_sclk_spi1,
	&clk_sclk_mmc0,
	&clk_sclk_mmc1,
	&clk_sclk_mmc2,
	&clk_sclk_irda,
	&clk_sclk_lcd,
	&clk_sclk_uhost,
	&clk_sclk_camif,
};

/* Clock initialisation the  source */
void  s3c_clksrc_init_parent(s3c_clk_t *clk, bool announce)
{
	struct clksource *srcs = clk->sources;
	u32 clksrc,mask;

	if (!srcs || !srcs->reg_src.reg) {
		if (!clk->parent)
			pr_debug("clock \"%s\": no parent clock specified\n",	clk->name);
		return;
	}

	mask = bit_mask(srcs->reg_src.shift, srcs->reg_src.size);
	clksrc = __raw_readl(srcs->reg_src.reg);
	clksrc &= mask;
	clksrc >>= srcs->reg_src.shift;

	if (!srcs->sources || !srcs->sources[clksrc]) {
		pr_debug("clock \"%s\": bad source %d\n",clk->name, clksrc);
		return;
	}

	clk->parent = srcs->sources[clksrc];

	if (announce)
		pr_notice("clock \"%s\": source \"%s\" from index(%d), rate is %ld\n",
		       clk->name, clk->parent->name, clksrc,s3c_clk_get_rate(clk));
}

#define GET_DIV(clk, field) ((((clk) & field##_MASK) >> field##_SHIFT) + 1)

void  s3c64xx_setup_clocks(void)
{
    s3c_clk_t *xtal_clk = &clk_xtal;
    unsigned long xtal;
    unsigned long apll;
    unsigned long mpll;
    unsigned long epll;
    unsigned long hclk;
    unsigned long hclk2;
    unsigned long pclk;
    unsigned int idx; 
    u32 clkdiv0;

    clkdiv0 = __raw_readl(S3C_CLK_DIV0);
    pr_debug( "%s: clkdiv0 = %08x\n", __func__, clkdiv0);

    xtal = s3c_clk_get_rate(xtal_clk);

    pr_debug( "%s: xtal is %ld\n", __func__, xtal);

    /* For now assume the mux always selects the crystal */
    epll = s3c_get_pll6553x(xtal, __raw_readl(S3C_EPLL_CON0),
                __raw_readl(S3C_EPLL_CON1));
    mpll = s3c_get_pll(xtal, __raw_readl(S3C_MPLL_CON));
    apll = s3c_get_pll(xtal, __raw_readl(S3C_APLL_CON));

    pr_debug("S3C64XX: PLL settings, A=%ld, M=%ld, E=%ld\n",
           apll, mpll, epll);

    if(__raw_readl(S3C_OTHERS) & S3C64XX_OTHERS_SYNCMUXSEL) {
        /* Synchronous mode */
        hclk2 = apll / GET_DIV(clkdiv0, S3C64XX_CLKDIV0_HCLK2);
        clk_h2.parent = &clk_mout_apll;
    }else {
        /* Asynchronous mode */
        hclk2 = mpll / GET_DIV(clkdiv0, S3C64XX_CLKDIV0_HCLK2);
        clk_h2.parent = &clk_mout_mpll;
    }

    hclk = hclk2 / GET_DIV(clkdiv0, S3C64XX_CLKDIV0_HCLK);
    pclk = hclk2 / GET_DIV(clkdiv0, S3C64XX_CLKDIV0_PCLK);

    pr_debug("S3C64XX: HCLK2=%ld, HCLK=%ld, PCLK=%ld\n",
           hclk2, hclk, pclk);

    clk_fout_mpll.rate = mpll;
    clk_fout_epll.rate = epll;
    clk_fout_apll.rate = apll;

    clk_h2.rate = hclk2;
    clk_h.rate = hclk;
    clk_p.rate = pclk;

    for (idx = 0; idx < ARRAY_SIZE(listptr_clk_parents); idx++)
        s3c_clksrc_init_parent(listptr_clk_parents[idx], false);
	
    for (idx = 0; idx < ARRAY_SIZE(listptr_clksrc_others_mulsrc); idx++)
        s3c_clksrc_init_parent(&listptr_clksrc_others_mulsrc[idx]->clk, false);
}

static unsigned long _s3c_clksrc_getrate(s3c_clk_t *clk)
{
	struct s3c64xx_clksrc *sclk = to_clksrc(clk);
	unsigned long rate = s3c_clk_get_rate(clk->parent);
	u32 clkdiv = __raw_readl(sclk->reg_div.reg);
	u32 mask = bit_mask(sclk->reg_div.shift, sclk->reg_div.size);

	clkdiv &= mask;
	clkdiv >>= sclk->reg_div.shift;
	clkdiv++;

	rate /= clkdiv;
	return rate;
}

static int _s3c_clksrc_setrate(s3c_clk_t *clk, unsigned long rate)
{
	struct s3c64xx_clksrc *sclk = to_clksrc(clk);
	void __iomem *reg = sclk->reg_div.reg;
	unsigned int div;
	u32 mask = bit_mask(sclk->reg_div.shift, sclk->reg_div.size);
	u32 val;

	if(clk->usage)
		pr_info("Warning: clock \"%s\" has been used, usage count: %d\n",
		         clk->name,clk->usage);

	rate = s3c_clk_round_rate(clk, rate);
	div = s3c_clk_get_rate(clk->parent) / rate;
	if (div > (1 << sclk->reg_div.size))
		return -EINVAL;

	val = __raw_readl(reg);
	val &= ~mask;
	val |= (div - 1) << sclk->reg_div.shift;
	__raw_writel(val, reg);

	return 0;
}

static int _s3c_clksrc_setparent(s3c_clk_t *clk, s3c_clk_t *parent)
{
	struct clksource *srcs = clk->sources;
	u32 clksrc,mask;
	int src_nr = -1;
	int idx;

	if(!srcs)  return -EINVAL;

	clksrc = __raw_readl(srcs->reg_src.reg);
	mask = bit_mask(srcs->reg_src.shift, srcs->reg_src.size);
	for (idx = 0; idx < srcs->nr_sources; idx++)
		if (srcs->sources[idx] == parent) {
			src_nr = idx;
			break;
		}

	if (src_nr >= 0) {
		clk->parent = parent;

		clksrc &= ~mask;
		clksrc |= src_nr << srcs->reg_src.shift;

		__raw_writel(clksrc, srcs->reg_src.reg);
		return 0;
	}

	return -EINVAL;
}

static int _s3c_clksrc_enable(s3c_clk_t *clk, int enable)
{
	struct s3c64xx_clksrc *sclk = to_clksrc(clk);
	if(sclk->enable) {
		return sclk->enable(clk,enable);
	}
	return 0;
}

static unsigned long _s3c_clksrc_roundrate(s3c_clk_t *clk, unsigned long rate)
{
	struct s3c64xx_clksrc *sclk = to_clksrc(clk);
	unsigned long parent_rate = s3c_clk_get_rate(clk->parent);
	int max_div = 1 << sclk->reg_div.size;
	int div;

	if (rate >= parent_rate)
		rate = parent_rate;
	else {
		div = parent_rate / rate;
		if (parent_rate % rate)
			div++;

		if (div == 0)
			div = 1;
		if (div > max_div)
			div = max_div;

		rate = parent_rate / div;
	}

	return rate;
}

static struct s3c_clk_ops clksrc_ops = {
	.enable  	= _s3c_clksrc_enable,
	.set_parent	= _s3c_clksrc_setparent,
	.get_rate	= _s3c_clksrc_getrate,
	.set_rate	= _s3c_clksrc_setrate,
	.round_rate	= _s3c_clksrc_roundrate,
};

static struct s3c_clk_ops clksrc_ops_nodiv = {
	.enable  	= _s3c_clksrc_enable,
	.set_parent	= _s3c_clksrc_setparent,
};

static struct s3c_clk_ops clksrc_ops_nosrc = {
	.enable  	= _s3c_clksrc_enable,
	.get_rate	= _s3c_clksrc_getrate,
	.set_rate	= _s3c_clksrc_setrate,
	.round_rate	= _s3c_clksrc_roundrate,
};

void  s3c_register_clksrc(struct s3c64xx_clksrc *clkp, 
						struct s3c64xx_clksrc *clkpp[],int size)
{
	struct s3c64xx_clksrc *clksrc;
	int idx;

	for (idx=0; idx<size ; idx++) {
		clksrc = clkp ? (clkp+idx) : clkpp[idx];

		if (!clksrc->reg_div.reg && !clksrc->clk.sources)
			pr_debug("%s: clock \"%s\" has no registers set\n",
			       __func__, clksrc->clk.name);

		/* fill in the default functions */

		if (!clksrc->clk.ops) {
			if (!clksrc->reg_div.reg)
				clksrc->clk.ops = &clksrc_ops_nodiv;
			else if (!clksrc->clk.sources)
				clksrc->clk.ops = &clksrc_ops_nosrc;
			else
				clksrc->clk.ops = &clksrc_ops;
		}

		/* setup the clocksource, but do not announce it
		 * as it may be re-set by the setup routines
		 * called after the rest of the clocks have been
		 * registered
		 */
		s3c_clksrc_init_parent(&clksrc->clk, false);

		s3c_clklist_link_add(&clksrc->clk);

	}
}

/**
 * s3c_disable_clksrc() - disable an array of clocks
 * @clkp: Pointer to the first clock in the array.
 * @clkpp: Pointer to thr first clock of the point array
 * @nr_clks: Number of clocks to register.
 *
 * for internal use only at initialisation time. disable the clocks in the
 * @clkp array.
 */

void __init s3c_disable_clksrc(struct s3c64xx_clksrc *clkp, 
							struct s3c64xx_clksrc *clkpp[],int nr_clks)
{
	struct s3c64xx_clksrc *clksrc;
	int idx;
	for (idx=0; idx < nr_clks; idx++) {
		clksrc = clkp ? (clkp+idx) : clkpp[idx];
		clksrc->enable(&clksrc->clk, 0);
	}
}

#ifdef USING_CLKSRC_LINK
#define  PRIV_ARRAY_SZ	(ARRAY_SIZE(listptr_clk_parents) + \
					ARRAY_SIZE(list_clksrc_init_on) + \
					ARRAY_SIZE(list_clksrc_init_off) + \
					ARRAY_SIZE(listptr_clksrc_others_mulsrc) + \
					ARRAY_SIZE(listptr_clksrc_others_singlesrc))

#define  PRIV_ALLOC_SZ sizeof(struct clksrc_link) * PRIV_ARRAY_SZ
static int s3c_clksrc_link_init(struct udevice *dev)
{
	struct clksrc_link *priv = dev_get_priv(dev);
	int idx;

	if(!priv)  return -ENOMEM;
	
	s3c64xx_clklist_link_head=priv;
	for(idx=1; idx < PRIV_ARRAY_SZ;idx++,priv++) {
		priv->clk=NULL;
		priv->next=&priv[1];
	}
	priv->next= NULL;

	return 0;
}
#else  /* ! USING_CLKSRC_LINK */
#define  PRIV_ALLOC_SZ  (sizeof(struct clksrc_array))
static int s3c_clksrc_link_init(struct udevice *dev)
{
	void *priv = dev_get_priv(dev);

	if(!priv)  return -ENOMEM;

	memset(priv,0,PRIV_ALLOC_SZ);
	s3c64xx_clklist_head=(struct clksrc_array *)priv;
	return 0;
}
#endif /* USING_CLKSRC_LINK */

static int s3c64xx_clk_probe(struct udevice *dev)
{
	int idx;

	//dev_info(dev,"%s,probe...\n",__func__);
	if(s3c_clksrc_link_init(dev)) {
		return -1;
	}
	
	s3c64xx_setup_clocks();

	for(idx=0;idx<ARRAY_SIZE(listptr_clk_parents);idx++)
		s3c_clklist_link_add(listptr_clk_parents[idx]);
	s3c_register_clksrc(list_clksrc_init_on, NULL, ARRAY_SIZE(list_clksrc_init_on));
	
	s3c_register_clksrc(list_clksrc_init_off, NULL, ARRAY_SIZE(list_clksrc_init_off));
	s3c_disable_clksrc(list_clksrc_init_off, NULL, ARRAY_SIZE(list_clksrc_init_off));
	
	s3c_register_clksrc(NULL, listptr_clksrc_others_mulsrc, ARRAY_SIZE(listptr_clksrc_others_mulsrc));
	s3c_disable_clksrc(NULL, listptr_clksrc_others_mulsrc, ARRAY_SIZE(listptr_clksrc_others_mulsrc));

	s3c_register_clksrc(NULL, listptr_clksrc_others_singlesrc, ARRAY_SIZE(listptr_clksrc_others_singlesrc));
	s3c_disable_clksrc(NULL, listptr_clksrc_others_singlesrc, ARRAY_SIZE(listptr_clksrc_others_singlesrc));
	
    for (idx = 0; idx < ARRAY_SIZE(list_clksrc_init_on); idx++)
        s3c_clk_enable(&list_clksrc_init_on[idx].clk, true);

#ifndef CFG_CLKSRC_CLKUART
	s3c_clk_enable(s3c_clk_get_by_id(CLK_ID_SCLK_UART), false);  /* clkuart is not selected for UART*/
#endif

	return 0;
}

static int s3c64xx_clk_bind(struct udevice *dev)
{
	//dev_info(dev,"dev(%s) bind!\n",dev->name);
	return 0;
}

static int s3c64xx_clk_enable(struct clk *clk)
{
	s3c_clk_t *clkp = s3c_clk_get_by_id(clk->id);
	pr_debug("%s, clock \"%s\",called by dev(%s)\n",__func__,
			clkp->name,clk->dev->name);
	if(!clkp)  return -EINVAL;
	
	return s3c_clk_enable(clkp,1);
}

static int s3c64xx_clk_disable(struct clk *clk)
{
	s3c_clk_t *clkp = s3c_clk_get_by_id(clk->id);
	pr_debug("%s, clock \"%s\",called by dev(%s)\n",__func__,
			clkp->name,clk->dev->name);
	if(!clkp)  return -EINVAL;
	
	return s3c_clk_enable(clkp,0);
}

static ulong s3c64xx_clk_get_rate(struct clk *clk)
{
	s3c_clk_t *clkp = s3c_clk_get_by_id(clk->id);
	pr_debug("%s, clock \"%s\",called by dev(%s)\n",__func__,
			clkp->name,clk->dev->name);
	if(!clkp)  return -EINVAL;
	
	return s3c_clk_get_rate(clkp);
}

static ulong s3c64xx_clk_set_rate(struct clk *clk, ulong rate)
{
	s3c_clk_t *clkp = s3c_clk_get_by_id(clk->id);
	pr_debug("%s, clock \"%s\",called by dev(%s)\n",__func__,
			clkp->name,clk->dev->name);
	if(!clkp)  return -EINVAL;
	
	return s3c_clk_set_rate(clkp,rate);
}

int s3c64xx_clk_set_parent(struct clk *clk, struct clk *parent)
{
	s3c_clk_t *clkp = s3c_clk_get_by_id(clk->id);
	s3c_clk_t *clkp_parent = s3c_clk_get_by_id(parent->id);
	pr_debug("%s, clock \"%s\", parent \"%s\", called by dev(%s)\n",__func__,
			clkp->name,clkp_parent->name, clk->dev->name);
	if(!clkp || !clkp_parent) 
		return -EINVAL;
	return s3c_clk_set_parent(clkp,clkp_parent);
}

static struct clk_ops s3c64xx_clk_ops = {
	.enable   = s3c64xx_clk_enable,
	.get_rate	= s3c64xx_clk_get_rate,
	.set_rate	= s3c64xx_clk_set_rate,
	.set_parent = s3c64xx_clk_set_parent,
	.disable  = s3c64xx_clk_disable,
};

static const struct udevice_id s3c_clk_ids[] = {
	{ .compatible = "samsung,s3c6410-clk" },
	{ }
};

/* clock module should be initialed ealier than others */
U_BOOT_DRIVER(ok6410_clock) = {
	.name     = "s3c6410_clock",
	.id          = UCLASS_CLK,
	.of_match = s3c_clk_ids,
	.probe     = s3c64xx_clk_probe,
	.ops       = &s3c64xx_clk_ops,
	.bind      = s3c64xx_clk_bind,
	.priv_auto_alloc_size = PRIV_ALLOC_SZ,
//	.ofdata_to_platdata = rk322x_clk_ofdata_to_platdata,
	.flags = DM_FLAG_PRE_RELOC,  /* funtion as "u-boot,dm-pre-reloc;" in dtsi */
};
/* ------------------------------------------------------------------------- */
/* NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * DoutAPLL(), get_HCLK(), get_PCLK() and MoutEPLL() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

#endif  /* CONFIG_IS_ENABLED(DM) */

static ulong get_PLLCLK(int pllreg)
{
	ulong r, m, p, s;

	if (pllreg == APLL)
		r = APLL_CON_REG;
	else if (pllreg == MPLL)
		r = MPLL_CON_REG;
	else if (pllreg == EPLL)
		r = EPLL_CON0_REG;
	else
		hang();

	m = (r>>16) & 0x3ff;
	p = (r>>8) & 0x3f;
	s = r & 0x7;

	return (m * (CONFIG_SYS_CLK_FREQ / (p * (1 << s))));
}

/* return ACLK frequency */
ulong get_DoutApll_CLK(void)
{
	if(CLK_SRC_REG & 1)
		return (	get_PLLCLK(APLL));
	else
		return CONFIG_SYS_CLK_FREQ;
}

/* return ACLK frequency */
ulong get_DoutMpll_CLK(void)
{
	if(CLK_SRC_REG & 2)
		return (	get_PLLCLK(MPLL));
	else
		return CONFIG_SYS_CLK_FREQ;
}

/* return MoutEPLL frequency */
ulong get_MoutEpll_CLK(void)
{
	if(CLK_SRC_REG & 1)
		return (get_PLLCLK(EPLL));
	else
		return CONFIG_SYS_CLK_FREQ;
}

/* return ARMCORE frequency */
ulong get_ARMCLK(void)
{
	ulong div;

	div = CLK_DIV0_REG;
	
	return (get_DoutApll_CLK() / ((div & 0x7) + 1));
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
	ulong fclk;

	uint hclkx2_div = ((CLK_DIV0_REG>>9) & 0x7) + 1;
	uint hclk_div = ((CLK_DIV0_REG>>8) & 0x1) + 1;
	
	if(OTHERS_REG & 0x40)
		fclk = get_DoutApll_CLK();		// SYNC Mode
	else
		fclk = get_DoutMpll_CLK();	// ASYNC Mode

	return fclk/(hclk_div * hclkx2_div);
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
	ulong fclk;
	uint hclkx2_div = ((CLK_DIV0_REG>>9) & 0x7) + 1;
	uint pre_div = ((CLK_DIV0_REG>>12) & 0xf) + 1;

	if(OTHERS_REG & 0x40) // SYNC MUX
		fclk = get_DoutApll_CLK();	// SYNC Mode
	else
		fclk = get_DoutMpll_CLK();	// ASYNC Mode

	return fclk/(hclkx2_div * pre_div);
}

unsigned long get_pwm_clk(void)
{
	return get_PCLK();
}

unsigned long get_uart_clk(int dev_index)
{
#if defined(CFG_CLKSRC_CLKUART)/* From EXT_UCLK1 */
	ulong freq,div;
	if(CLK_SRC_REG & (1<<13)) {
		 if (CLK_DIV0_REG & 0x10)
			freq = get_DoutMpll_CLK()/2;
		else
			freq = get_DoutMpll_CLK();
	}else
		freq = get_MoutEpll_CLK();
	div = (CLK_DIV2_REG >>16) & 0xF;
	
	freq =  freq/(div+1); 
	return freq;
#else  /* From PCLK */
	return get_PCLK();
#endif
}

unsigned long get_arm_clk(void)
{
	return get_ARMCLK();
}

int print_cpuinfo(void)
{
	printf("****************************************\r\n");
	//printf("**    u-boot 201809                   **\r\n");
	printf("**    Updated for OK6410A Board        **\r\n");
	printf("**    Version 1.0 (2018/10/13)        **\r\n");
	printf("**    OEM: Golden Creation            **\r\n");
	printf("****************************************\r\n");

	printf("\nCPU:  S3C6410 @%dMHz\n", (int)(get_ARMCLK()/1000000));
	printf("        Mclk = %dMHz,  Eclk = %dMHz\n",
			(int)(get_DoutMpll_CLK()/1000000),
			(int)(get_MoutEpll_CLK()/1000000));
	printf("        Hclk = %dMHz,  Pclk = %dMHz\n        ",
			(int)(get_HCLK()/1000000), 
			(int)(get_PCLK()/1000000));

/**************
* Display Serial SRC
***************/

#if defined(CFG_CLKSRC_CLKUART)
	puts("Serial-source = CLKUART ");
#else
	puts("Serial-source = PCLK ");
#endif

	if(OTHERS_REG & 0x80)
		printf("(SYNC Mode)\n");
	else
		printf("(ASYNC Mode)\n");

	return 0;
}
