// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm/device-internal.h>
#include <asm/arch/gpio-regs.h>
#include <asm/arch/gpio-func.h>

DECLARE_GLOBAL_DATA_PTR;

//#define  pr_debug  pr_err  //for debug

typedef struct s3c64xx_bank_info {
	gpio_bank_t *reg;
	unsigned int  gpio_count;
	unsigned int  flags;
} s3c64xx_bank_info_t;


/* Platform data for each bank */
struct s3c64xx_gpio_platdata {
      /* struct s3c64xx_bank_info */
	gpio_bank_t *bank;
	unsigned int  gpio_count;
	unsigned int  flags;
	
	const char *bank_name;	/* Name of port, e.g. 'gpa' 'gpb'" */
	unsigned int gpio_base;
};


int s3c_gpio_setvalue(struct s3c64xx_bank_info *base,unsigned int off, unsigned int val)
{
	S3C64XX_REG32 *reg = & base->reg->dat;
	u32 dat;
	
	if(base->flags & GPIO_FLAGS_CON_DOUBLE)
		reg += 1;
	
	dat = __raw_readl(reg);
	dat &= ~(1 << off);
	dat |= val << off;
	__raw_writel(dat, reg);

	return 0;
}

unsigned int s3c_gpio_getvalue(struct s3c64xx_bank_info *base, unsigned int off)
{
	S3C64XX_REG32 *reg = & base->reg->pud;
	u32 dat;

	if(base->flags & GPIO_FLAGS_CON_DOUBLE)
		reg += 1;

	dat = __raw_readl(reg);
	dat >>= off;
	dat &= 1;

	return (unsigned int)dat;
}

int s3c_gpio_setpull(struct s3c64xx_bank_info *base, unsigned int off, unsigned int pull)
{
	S3C64XX_REG32 *reg = & base->reg->pud;
	int shift = off * 2;
	u32 pup;
	
	if(base->flags & GPIO_FLAGS_CON_DOUBLE)
		reg += 1;
	
	pup = __raw_readl(reg);
	pup &= ~(3 << shift);
	pup |= pull << shift;
	__raw_writel(pup, reg);

	return 0;
}

unsigned int s3c_gpio_getpull(struct s3c64xx_bank_info *base, unsigned int off)
{
	S3C64XX_REG32 *reg = & base->reg->pud;
	int shift = off * 2;
	u32 pup;

	if(base->flags & GPIO_FLAGS_CON_DOUBLE)
		reg += 1;

	pup = __raw_readl(reg);
	pup >>= shift;
	pup &= 0x3;

	return (unsigned int)pup;
}

/*
 * s3c_gpio_setcfg_2bit - Samsung 2bit style GPIO configuration.
 * @chip: The gpio chip that is being configured.
 * @off: The offset for the GPIO being configured.
 * @cfg: The configuration value to set.
 *
 * This helper deal with the GPIO cases where the control register
 * has two bits of configuration per gpio, which have the following
 * functions:
 *	00 = input
 *	01 = output
 *	1x = special function
 */

static int _s3c_gpio_setcfg_2bit(gpio_bank_t * base, unsigned int off, unsigned int cfg)
{
	S3C64XX_REG32 *reg = & base->con;
	unsigned int shift = off * 2;
	u32 con;

	cfg = (cfg & 0x3) <<shift;

	con = __raw_readl(reg);
	con &= ~(0x3 << shift);
	con |= cfg;
	__raw_writel(con, reg);

	return 0;
}

/*
 * s3c_gpio_setcfg_4bit - Samsung 4bit single register GPIO config.
 * @chip: The gpio chip that is being configured.
 * @off: The offset for the GPIO being configured.
 * @cfg: The configuration value to set.
 *
 * This helper deal with the GPIO cases where the control register has 4 bits
 * of control per GPIO, generally in the form of:
 *	0000 = Input
 *	0001 = Output
 *	others = Special functions (dependent on bank)
 *
 * Note, since the code to deal with the case where there are two control
 * registers instead of one, we do not have a separate set of functions for
 * each case.
 */

static int _s3c_gpio_setcfg_4bit(gpio_bank_t * base, unsigned int off, unsigned int cfg)
{
	S3C64XX_REG32 *reg = & base->con;
	unsigned int shift = (off & 7) * 4;
	u32 con;

	if (off >= 8 )	reg += 1;

	cfg = (cfg &0xf) << shift;
	
	con = __raw_readl(reg);
	con &= ~(0xf << shift);
	con |= cfg;
	__raw_writel(con, reg);

	return 0;
}

int s3c_gpio_setcfg(struct s3c64xx_bank_info * base, unsigned int off, unsigned int cfg)
{
	if(base->flags & GPIO_FLAGS_CFG_4BIT)
		_s3c_gpio_setcfg_4bit(base->reg, off, cfg);
	else if(base->flags & GPIO_FLAGS_CFG_2BIT)
		_s3c_gpio_setcfg_2bit(base->reg, off, cfg);
	else {
		printf(KERN_ERR "Error: unknow setcfg function!base flags: %#x\n",base->flags);
		return -1;
	}

	return 0;
}

/*
 * _s3c_gpio_getcfg_2bit - Samsung 2bit style GPIO configuration read.
 * @chip: The gpio chip that is being configured.
 * @off: The offset for the GPIO being configured.
 *
 * The reverse of _s3c_gpio_getcfg_2bit(). Will return a value which
 * could be directly passed back to samsung_gpio_setcfg_2bit(), from the
 * S3C_GPIO_SPECIAL() macro.
 */

static unsigned int _s3c_gpio_getcfg_2bit(gpio_bank_t *base, unsigned int off)
{
	S3C64XX_REG32 *reg = & base->con;
	u32 con;

	con = __raw_readl(reg);
	con >>= off * 2;
	con &= 3;

	return (unsigned int)con;
}

/*
 * _s3c_gpio_getcfg_4bit - Samsung 4bit single register GPIO config read.
 * @chip: The gpio chip that is being configured.
 * @off: The offset for the GPIO being configured.
 *
 * The reverse of s3c_gpio_setcfg_4bit(), turning a gpio configuration
 * register setting into a value the software can use, such as could be passed
 * to samsung_gpio_setcfg_4bit().
 *
 * @sa samsung_gpio_getcfg_2bit
 */

static unsigned int _s3c_gpio_getcfg_4bit(gpio_bank_t *base, unsigned int off)
{
	S3C64XX_REG32 *reg = & base->con;
	unsigned int shift = (off & 7) * 4;
	u32 con;

	if (off >= 8 )	reg += 1;

	con = __raw_readl(reg);
	con >>= shift;
	con &= 0xf;

	/* this conversion works for IN and OUT as well as special mode */
	return (unsigned int)con;
}
int s3c_gpio_getcfg(struct s3c64xx_bank_info *base,unsigned int off)
{
	unsigned int ret=0;
	if(base->flags & GPIO_FLAGS_CFG_4BIT)
		ret =  _s3c_gpio_getcfg_4bit(base->reg, off);
	else if(base->flags & GPIO_FLAGS_CFG_2BIT)
		ret =  _s3c_gpio_getcfg_2bit(base->reg, off);
	else {
		printf(KERN_ERR "Error: unknow getcfg function!\n");
		ret = -1;
	}

	return ret;
}

int  inline s3c_gpio_input(struct s3c64xx_bank_info * base, unsigned int off)
{
	s3c_gpio_setcfg(base, off , S3C_GPIO_INPUT);
	return 0;
}

int inline s3c_gpio_output(struct s3c64xx_bank_info * base, unsigned int off, unsigned int dat)
{
	s3c_gpio_setcfg(base, off , S3C_GPIO_OUTPUT);
	s3c_gpio_setvalue(base, off, dat);
	return 0;
}

static int gpio_to_udevice (unsigned int gpio, struct udevice  **dev)
{
	struct gpio_desc desc;
	int ret= gpio_to_device(gpio, &desc);
	if(ret)
		return ret ;

	*dev= desc.dev;
	return 0;
}

#if CONFIG_IS_ENABLED(DM_GPIO)
int s3c64xx_gpio_set_cfg(unsigned gpio, int cfg) 
{
	struct udevice *udev;


	struct s3c64xx_gpio_platdata *plat;
	unsigned offset;

	if(gpio_to_udevice(gpio, &udev)) {
		pr_err("Error: invalid gpio to get udevice!\n");
		return -1;
	}
	
	plat = dev_get_platdata(udev);
	offset = gpio - plat->gpio_base;
	pr_debug("%s -- dev(%s) offset: %d, cfg: %#x\n", __func__,
				udev->name,gpio -plat->gpio_base, cfg);
	return s3c_gpio_setcfg((struct s3c64xx_bank_info *)plat, offset, cfg);
}

int s3c64xx_gpio_get_cfg(unsigned gpio) 
{
	struct udevice *udev;
	struct s3c64xx_gpio_platdata *plat;
	unsigned offset;

	if(gpio_to_udevice(gpio, &udev)) {
		pr_err("Error: invalid gpio to get udevice!\n");
		return -1;
	}
	
	plat = dev_get_platdata(udev);
	offset = gpio - plat->gpio_base;
	pr_debug("%s -- dev(%s) offset: %d\n", __func__,
				udev->name,gpio -plat->gpio_base);
	return s3c_gpio_getcfg((struct s3c64xx_bank_info *)plat, offset);
}

int s3c64xx_gpio_set_pull(unsigned gpio, int mode)
{
	struct udevice *udev;
	struct s3c64xx_gpio_platdata *plat;
	unsigned offset;

	if(gpio_to_udevice(gpio, &udev)) {
		pr_err("Error: invalid gpio to get udevice!\n");
		return -1;
	}
	
	plat = dev_get_platdata(udev);
	offset = gpio - plat->gpio_base;
	pr_debug("%s -- dev(%s) offset: %d, mode: %#x\n", __func__,
				udev->name,gpio -plat->gpio_base, mode);
	return s3c_gpio_setpull((struct s3c64xx_bank_info *)plat, offset,mode);
}

int s3c64xx_gpio_set_value(unsigned gpio, int val)
{
	struct udevice *udev;
	struct s3c64xx_gpio_platdata *plat;
	unsigned offset;

	if(gpio_to_udevice(gpio, &udev)) {
		pr_err("Error: invalid gpio to get udevice!\n");
		return -1;
	}
	
	plat = dev_get_platdata(udev);
	offset = gpio - plat->gpio_base;
	pr_debug("%s -- dev(%s) offset: %d, val: %#x\n", __func__,
				udev->name,gpio -plat->gpio_base, val);
	return s3c_gpio_setvalue((struct s3c64xx_bank_info *)plat, offset,val);
}

int s3c64xx_gpio_get_value(unsigned gpio)
{
	struct udevice *udev;
	struct s3c64xx_gpio_platdata *plat;
	unsigned offset;

	if(gpio_to_udevice(gpio, &udev)) {
		pr_err("Error: invalid gpio to get udevice!\n");
		return -1;
	}
	
	plat = dev_get_platdata(udev);
	offset = gpio - plat->gpio_base;
	pr_debug("%s -- dev(%s) offset: %d\n", __func__,
				udev->name,gpio -plat->gpio_base);
	return s3c_gpio_getvalue((struct s3c64xx_bank_info *)plat, offset);
}

#else

static struct s3c64xx_bank_info *s3c_gpio_get_bank(unsigned int gpio)
{
	static  struct s3c64xx_bank_info gp_bank;
	int bank_nr = GPIO_GET_BANK(gpio);
	gpio_bank_t *bank_base = (gpio_bank_t *)S3C64XX_GetBase_GPIO();

	if(bank_nr >= S3C64XX_BANK_NR_MAX)
		return NULL ;  /* invalid gpio bank */
	
	gp_bank.flags = GPIO_GET_FLAGS(gpio);
	gp_bank.gpio_count = GPIO_GET_FLAGS(gpio);
	gp_bank.reg  = bank_base + bank_nr;

	return &gp_bank;
}

/* Common GPIO API - SPL does not support driver model yet */
int s3c64xx_gpio_set_cfg(unsigned gpio, int cfg)
{
	struct s3c64xx_bank_info *bank_info;
	unsigned int offset = GPIO_GET_OFFSET(gpio);

	bank_info=s3c_gpio_get_bank(gpio);
	if(!bank_info) return -1;

	if(offset >= bank_info->gpio_count)
		return -1; /* invalid gpio number in bank*/

	return s3c_gpio_setcfg(bank_info, offset, cfg);
}

/* Common GPIO API - SPL does not support driver model yet */
int s3c64xx_gpio_get_cfg(unsigned gpio) 
{
	struct s3c64xx_bank_info *bank_info;
	unsigned int offset = GPIO_GET_OFFSET(gpio);
	
	bank_info=s3c_gpio_get_bank(gpio);
	if(!bank_info) return -1;

	if(offset >= bank_info->gpio_count)
		return -1; /* invalid gpio number in bank*/

	return s3c_gpio_getcfg(bank_info, offset);
}

int s3c64xx_gpio_set_pull(unsigned gpio, int mode) 
{
	struct s3c64xx_bank_info *bank_info;
	unsigned int offset = GPIO_GET_OFFSET(gpio);
	
	bank_info=s3c_gpio_get_bank(gpio);
	if(!bank_info) return -1;

	if(offset >= bank_info->gpio_count)
		return -1; /* invalid gpio number in bank*/

	return s3c_gpio_setpull(bank_info, offset, mode);
}

/* Common GPIO API - SPL does not support driver model yet */
int s3c64xx_gpio_set_value(unsigned gpio, int value)
{
	struct s3c64xx_bank_info *bank_info;
	unsigned int offset = GPIO_GET_OFFSET(gpio);

	bank_info=s3c_gpio_get_bank(gpio);
	if(!bank_info) return -1;

	if(offset >= bank_info->gpio_count)
		return -1; /* invalid gpio number in bank*/
	
	return s3c_gpio_setvalue(bank_info, offset, value);
}

/* Common GPIO API - SPL does not support driver model yet */
int s3c64xx_gpio_get_value(unsigned gpio) 
{
	struct s3c64xx_bank_info *bank_info;
	unsigned int offset = GPIO_GET_OFFSET(gpio);
	
	bank_info=s3c_gpio_get_bank(gpio);
	if(!bank_info) return -1;

	if(offset >= bank_info->gpio_count)
		return -1; /* invalid gpio number in bank*/
	
	return s3c_gpio_getvalue(bank_info, offset);
}
#endif  /* CONFIG_IS_ENABLED(DM_GPIO) */


#if CONFIG_IS_ENABLED(DM_GPIO)

#if CONFIG_IS_ENABLED(OF_CONTROL)

/**
 * We have a top-level GPIO device with no actual GPIOs. It has a child
 * device for each Exynos GPIO bank.
 */
static int s3c64xx_gpio_bind(struct udevice *parent)
{
	struct s3c64xx_gpio_platdata *plat = parent->platdata;
	gpio_bank_t *bank, *base;
	const void *blob = gd->fdt_blob;
	int node;

	pr_debug("dev(%s), ofnode gpio bind!\n",parent->name);

	/* If this is a child device, there is nothing to do here */
	if (plat)
		return 0;

	base = (gpio_bank_t *)devfdt_get_addr(parent);
	for (node = fdt_first_subnode(blob, dev_of_offset(parent)), bank = base;
	     node > 0;
	     node = fdt_next_subnode(blob, node), bank++) {
		struct s3c64xx_gpio_platdata *plat;
		struct udevice *dev;
		fdt_addr_t reg;
		int ret;

		if (!fdtdec_get_bool(blob, node, "gpio-controller"))
			continue;
		plat = calloc(1, sizeof(*plat));
		if (!plat)
			return -ENOMEM;

		plat->bank_name = fdt_get_name(blob, node, NULL);
		plat->flags  =  fdtdec_get_int(blob, node, "gpio-bank-flags", 
			         GPIO_FLAGS_CFG_4BIT | GPIO_FLAGS_SLP_MODE);
		plat->gpio_count =  fdtdec_get_int(blob, node, "gpio-bank-count", 8);
		plat->gpio_base =  fdtdec_get_int(blob, node, "gpio-nr-base", 0);

		pr_debug("%s, gpio base: %d count: %d flags: %#x\n", plat->bank_name,
						plat->gpio_base ,plat->gpio_count,plat->flags);
		ret = device_bind(parent, parent->driver,
				  plat->bank_name, plat, -1, &dev);
		if (ret)
			return ret;

		dev_set_of_offset(dev, node);

		reg = devfdt_get_addr(dev);
		if (reg != FDT_ADDR_T_NONE)
			bank = (gpio_bank_t *)((ulong)base + reg);

		plat->bank = bank;

		debug("dev at %p: %s\n", bank, plat->bank_name);
	}

	return 0;
}

#else

/*
 * GPIO bank summary:
 *
 * Bank	GPIOs	Style	SlpCon	ExtInt Group
 * A          8           4Bit        Yes         1
 * B          7           4Bit        Yes         1
 * C          8           4Bit        Yes         2
 * D          5           4Bit        Yes         3
 * E          5           4Bit        Yes         None
 * F          16          2Bit        Yes         4 [1]
 * G          7           4Bit        Yes         5
 * H          10         4Bit[2]    Yes         6
 * I          16          2Bit        Yes         None
 * J          12          2Bit        Yes         None
 * K          16          4Bit[2]    No         None
 * L          15          4Bit[2]    No         None
 * M          6           4Bit        No         IRQ_EINT
 * N          16          2Bit       No         IRQ_EINT
 * O          16          2Bit       Yes         7
 * P          15          2Bit        Yes         8
 * Q          9           2Bit        Yes         9
 *
 * [1] BANKF pins 14,15 do not form part of the external interrupt sources
 * [2] BANK has two control registers, GPxCON0 and GPxCON1
 */

const struct s3c64xx_gpio_platdata s3c64xx_gpios[] = {
	{    /* GPA */
		.bank_name = "gpa",
		.reg	= (gpio_bank_t *)S3C64XX_GPA_BASE,
		.flags = S3C64XX_GPIO_A_FLAGS  ,
		.gpio_count = S3C64XX_GPIO_A_NR, 
		.gpio_base = S3C64XX_GPIO_A_START
	}, {/* GPB */
		.bank_name = "gpb",
		.reg	= (gpio_bank_t *)S3C64XX_GPB_BASE,
		.flags = S3C64XX_GPIO_B_FLAGS ,
		.gpio_count = S3C64XX_GPIO_B_NR, 
		.gpio_base = S3C64XX_GPIO_B_START
	}, {/* GPC */
		.bank_name = "gpc",
		.reg	= (gpio_bank_t *)S3C64XX_GPC_BASE,
		.flags = S3C64XX_GPIO_C_FLAGS ,
		.gpio_count = S3C64XX_GPIO_C_NR, 
		.gpio_base = S3C64XX_GPIO_C_START
	}, {/* GPD */
		.bank_name = "gpd",
		.reg	= (gpio_bank_t *)S3C64XX_GPD_BASE,
		.flags = S3C64XX_GPIO_D_FLAGS ,
		.gpio_count = S3C64XX_GPIO_D_NR, 
		.gpio_base = S3C64XX_GPIO_D_START
	}, {/* GPE */
		.bank_name = "gpe",
		.reg	= (gpio_bank_t *)S3C64XX_GPE_BASE,
		.flags = S3C64XX_GPIO_E_FLAGS ,
		.gpio_count = S3C64XX_GPIO_E_NR,
		.gpio_base = S3C64XX_GPIO_E_START
	}, {/* GPF */
		.bank_name = "gpf",
		.reg	= (gpio_bank_t *)S3C64XX_GPF_BASE,
		.flags = S3C64XX_GPIO_F_FLAGS,
		.gpio_count = S3C64XX_GPIO_F_NR,
		.gpio_base = S3C64XX_GPIO_F_START
	}, {/* GPG */
		.bank_name = "gpg",
		.reg	= (gpio_bank_t *)S3C64XX_GPG_BASE, 
		.flags = S3C64XX_GPIO_G_FLAGS,
		.gpio_count = S3C64XX_GPIO_G_NR,
		.gpio_base = S3C64XX_GPIO_G_START
	}, {/* GPH */ 
		.bank_name = "gph",
		.reg	= (gpio_bank_t *)S3C64XX_GPH_BASE,
		.flags = S3C64XX_GPIO_H_FLAGS ,
		.gpio_count = S3C64XX_GPIO_H_NR,
		.gpio_base = S3C64XX_GPIO_H_START
	}, {/* GPI */
		.bank_name = "gpi", 
		.reg	= (gpio_bank_t *)S3C64XX_GPI_BASE,
		.flags = S3C64XX_GPIO_I_FLAGS ,
		.gpio_count = S3C64XX_GPIO_I_NR,
		.gpio_base = S3C64XX_GPIO_I_START
	}, {/* GPJ */
		.bank_name = "gpj",
		.reg	= (gpio_bank_t *)S3C64XX_GPJ_BASE,
		.flags = S3C64XX_GPIO_J_FLAGS ,
		.gpio_count = S3C64XX_GPIO_J_NR,
		.gpio_base = S3C64XX_GPIO_J_START
	}, {/* GPK */
		.bank_name = "gpk",
		.reg	= (gpio_bank_t *)S3C64XX_GPK_BASE,
		.flags = S3C64XX_GPIO_K_FLAGS ,
		.gpio_count = S3C64XX_GPIO_K_NR,
		.gpio_base = S3C64XX_GPIO_K_START
	}, {/* GPL */
		.bank_name = "gpl",
		.reg	= (gpio_bank_t *)S3C64XX_GPL_BASE,
		.flags = S3C64XX_GPIO_L_FLAGS ,
		.gpio_count = S3C64XX_GPIO_L_NR,
		.gpio_base = S3C64XX_GPIO_L_START
	}, {/* GPM */
		.bank_name = "gpm",
		.reg	= (gpio_bank_t *)S3C64XX_GPM_BASE,
		.flags =  S3C64XX_GPIO_M_FLAGS ,
		.gpio_count = S3C64XX_GPIO_M_NR,
		.gpio_base = S3C64XX_GPIO_M_START
	}, {/* GPN */
		.bank_name = "gpn",
		.reg	= (gpio_bank_t *)S3C64XX_GPN_BASE,
		.flags =  S3C64XX_GPIO_N_FLAGS ,
		.gpio_count = S3C64XX_GPIO_N_NR,
		.gpio_base = S3C64XX_GPIO_N_START
	}, {/* GPO */
		.bank_name = "gpo",
		.reg	= (gpio_bank_t *)S3C64XX_GPO_BASE,
		.flags =  S3C64XX_GPIO_O_FLAGS ,
		.gpio_count = S3C64XX_GPIO_O_NR,
		.gpio_base = S3C64XX_GPIO_O_START
	}, {/* GPP */
		.bank_name = "gpp",
		.reg	= (gpio_bank_t *)S3C64XX_GPP_BASE,
		.flags = S3C64XX_GPIO_P_FLAGS ,
		.gpio_count = S3C64XX_GPIO_P_NR,
		.gpio_base = S3C64XX_GPIO_P_START
	}, {/* GPQ */
		.bank_name = "gpq",
		.reg	= (gpio_bank_t *)S3C64XX_GPQ_BASE,
		.flags = S3C64XX_GPIO_Q_FLAGS ,
		.gpio_count = S3C64XX_GPIO_Q_NR,
		.gpio_base = S3C64XX_GPIO_Q_START
	}
};

static int s3c64xx_gpio_bind(struct udevice *parent)
{
	struct s3c64xx_gpio_platdata *plat = parent->platdata;
	struct udevice *dev;
	gpio_bank_t *bank, *base;
	int bank_nr,ret;

	pr_debug("dev(%s), gpio bind!\n",parent->name);
	/* If this is a child device, there is nothing to do here */
	if (plat)
		return 0;
	for ( plat = s3c64xx_gpios,bank_nr=ARRAY_SIZE(s3c64xx_gpios);
		bank_nr >0 ; bank_nr--,plat++)  {
			ret = device_bind(parent, parent->driver,
				  plat->bank_name, plat, -1, &dev);
			if(ret) return ret;
	}
	
	return 0;
}
#endif  /* OF_CONTROL */

/* Driver model interface */
/* set GPIO pin 'gpio' as an input */
static int s3c_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct s3c64xx_bank_info *state = dev_get_platdata(dev);

	/* Configure GPIO direction as input. */
	s3c_gpio_input(state, offset);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
static int s3c_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct s3c64xx_bank_info *state = dev_get_platdata(dev);

	/* Configure GPIO output value. */
	s3c_gpio_output(state, offset,value);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
static int s3c_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct s3c64xx_bank_info *state = dev_get_platdata(dev);

	return s3c_gpio_getvalue(state, offset);
}

/* write GPIO OUT value to pin 'gpio' */
static int s3c_gpio_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct s3c64xx_bank_info *state = dev_get_platdata(dev);

	s3c_gpio_setvalue(state, offset, value);

	return 0;
}
static int s3c_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct s3c64xx_bank_info *state = dev_get_platdata(dev);
	int cfg;

	cfg = s3c_gpio_getcfg(state, offset);
	if (cfg == S3C_GPIO_OUTPUT)
		return GPIOF_OUTPUT;
	else if (cfg == S3C_GPIO_INPUT)
		return GPIOF_INPUT;
	else
		return GPIOF_FUNC;
}

static const struct dm_gpio_ops gpio_s3c_ops = {
	.direction_input	= s3c_gpio_direction_input,
	.direction_output	= s3c_gpio_direction_output,
	.get_value		= s3c_gpio_get_value,
	.set_value		= s3c_gpio_set_value,
	.get_function		= s3c_gpio_get_function,
};

static int s3c64xx_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct s3c64xx_gpio_platdata *plat = dev_get_platdata(dev);
//	struct s3c64xx_bank_info *priv =dev_get_priv(dev);

	pr_debug("dev(%s): probe!\n",dev->name);
	/* Only child devices have ports */
	if (!plat)
		return 0;

//	pr_info("%s,priv=%p\n",  dev->name,priv);
//	if(priv == NULL)
//		priv = (struct s3c64xx_bank_info *)plat;

	uc_priv->gpio_count = plat->gpio_count;
	uc_priv->bank_name = plat->bank_name;
	uc_priv->gpio_base = plat->gpio_base;
	pr_debug("%s, gpio base: %d count: %d flags: %#x\n", 
			uc_priv->bank_name, uc_priv->gpio_base ,
			uc_priv->gpio_count,plat->flags);

	return 0;
}

static const struct udevice_id s3c64xx_gpio_ids[] = {
	{ .compatible = "samsung,s3c6410-pinctrl" },
	{ }
};

U_BOOT_DRIVER(s3c64xx_gpio) = {
	.name	= "s3c64xx_gpio",
	.id	= UCLASS_GPIO,
	.of_match = s3c64xx_gpio_ids,
	.bind	= s3c64xx_gpio_bind,
	.probe = s3c64xx_gpio_probe,
	//.priv_auto_alloc_size = sizeof(struct s3c64xx_bank_info), /* use dev->platdata ,instead */
	.ops	= &gpio_s3c_ops,
	.flags = DM_FLAG_PRE_RELOC,  /* funtion as "u-boot,dm-pre-reloc;" in dtsi */
};

#endif  /* CONFIG_IS_ENABLED(DM_GPIO) */
