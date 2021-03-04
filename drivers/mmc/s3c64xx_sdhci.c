// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 */
 
#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <fdtdec.h>
#include <clk.h>
#include <linux/libfdt.h>
#include <asm/gpio.h>
#include <asm/arch/sdhc.h>
#include <asm/arch/clk.h>
#include <asm/arch/periph.h>
#include <errno.h>
#include <asm/arch/clock-regs.h>
#include <asm/arch/gpio-func.h>

//#define  pr_debug  pr_err  //for debug

#if CONFIG_IS_ENABLED(DM_MMC)
struct s3c64xx_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

DECLARE_GLOBAL_DATA_PTR;
struct clk sdhc_clk;
#else
struct s3c_clk *sdhc_clkp;
#endif

static char *S3C_NAME = "S3C64XX_SDHCI";
static void s3c_sdhci_set_control_reg(struct sdhci_host *host)
{
	unsigned long val;
	/*
	 * SELCLKPADDS[17:16]
	 * 00 = 2mA
	 * 01 = 4mA
	 * 10 = 7mA
	 * 11 = 9mA
	 */
	sdhci_writel(host, HM_CTRL4_DRIVE_MASK(0x3), HM_CONTROL4);

	val = sdhci_readl(host, HM_CONTROL2);
	val &= HM_CTRL2_SELBASECLK_MASK(3); /* 00 or 01 =HCLK, 10=EPLL, 11=EXTCLK */
	val |=	HM_CTRL2_ENSTAASYNCCLR | /* write status clear async mode enable */
			HM_CTRL2_ENCMDCNFMSK |   /* command conflict mask enable */
			HM_CTRL2_ENFBCLKRX |     /* Feedback Clock Enable for Rx Clock */
			HM_CTRL2_ENCLKOUTHOLD;   /* SDCLK hold enable */
	sdhci_writel(host, val, HM_CONTROL2);

	/*
	 * FCSEL3[31] FCSEL2[23] FCSEL1[15] FCSEL0[7]
	 * FCSel[1:0] : Rx Feedback Clock Delay Control
	 *	Inverter delay means10ns delay if SDCLK 50MHz setting
	 *	01 = Delay1 (basic delay)
	 *	11 = Delay2 (basic delay + 2ns)
	 *	00 = Delay3 (inverter delay)
	 *	10 = Delay4 (inverter delay + 2ns)
	 */
	val = HM_CTRL3_FCSEL0 | HM_CTRL3_FCSEL1;
	sdhci_writel(host, val, HM_CONTROL3);

}

static void s3c_mmc_clock_enable (struct sdhci_host *host, int on)
{
	u16 reg16;

	if (on == 0) {
		sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);
	} else {
		reg16 = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
		sdhci_writew(host, reg16 | (0x1<<2), SDHCI_CLOCK_CONTROL);

		while (1) {
			reg16 = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
			if (reg16 & (0x1<<3))	/*  SD_CLKSRC is Stable */
				break;
		}

	}
}

static void s3c_set_clock(struct sdhci_host *host, u32 div)
{
	ulong i,clocksrc;
	uint clock=host->clock;

	/* ToDo : Use the Clock Framework */
#if CONFIG_IS_ENABLED(DM_MMC)
	clk_enable(&sdhc_clk);
	clocksrc=clk_get_rate(&sdhc_clk);
#else
	s3c_clk_enable(sdhc_clkp,1);
	clocksrc=s3c_clk_get_rate(sdhc_clkp);
#endif

	/*
	 * SELBASECLK[5:4]
	 * 00/01 = HCLK
	 * 10 = EPLL
	 * 11 = XTI or XEXTCLK
	 */
	i = sdhci_readl(host, HM_CONTROL2);
	i &= ~ HM_CTRL2_SELBASECLK_MASK(0x3);
	i |= HM_CTRL2_SELBASECLK_MASK(0x2);
	sdhci_writel(host, i, HM_CONTROL2);

	s3c_mmc_clock_enable(host,0);

	i = sdhci_readw(host, SDHCI_CLOCK_CONTROL) & ~(0xff << 8);
	sdhci_writew(host, i, SDHCI_CLOCK_CONTROL);

	pr_debug("%s, input div: %d\n", __func__,div);
	if(clock) {
		div = clocksrc/clock;
		if (div * clock != clocksrc) div++;
	}
	if(div>>8) div=256;
	while((clocksrc/div) > host->max_clk) div++;
	pr_info("%s, wanted: %d Hz, actual: %d Hz\n",
			__func__,clock,(uint)(clocksrc/div));

	/* SDCLK Value Setting + Internal Clock Enable */
	/*
	 * CLKCON
	 * SELFREQ[15:8]	: base clock divied by value
	 * ENSDCLK[2]		: SD Clock Enable
	 * STBLINTCLK[1]	: Internal Clock Stable
	 * ENINTCLK[0]		: Internal Clock Enable
	 */
	div >>= 1;
	sdhci_writew(host, ((div<<8) | 0x1), SDHCI_CLOCK_CONTROL);

	/* Wait max 10 ms */
	i = 10;
	while (!(sdhci_readw(host, SDHCI_CLOCK_CONTROL) & 0x2)) {
		if (i == 0) {
			printf("%s: SDHC Internel clock stabel timeout error\n", __func__);
			return;
		}
		i--;
		udelay(1000);
	}

	s3c_mmc_clock_enable(host,1);
	pr_debug("HM_CONTROL2(0x80) = 0x%08x\n", sdhci_readl(host, HM_CONTROL2));
	pr_debug("HM_CONTROL3(0x84) = 0x%08x\n", sdhci_readl(host, HM_CONTROL3));
	pr_debug("HM_CLKCON  (0x2c) = 0x%04x\n", sdhci_readw(host, HM_CLKCON));
}

static const struct sdhci_ops s3c_sdhci_ops = {
	.set_clock	= &s3c_set_clock,
	.set_control_reg = &s3c_sdhci_set_control_reg,
};

static int s3c_sdhci_core_init(struct sdhci_host *host)
{
	host->name = S3C_NAME;

	host->max_clk = 52000000;
	host->ops = &s3c_sdhci_ops;
	host->quirks = SDHCI_QUIRK_NO_HISPD_BIT | SDHCI_QUIRK_BROKEN_VOLTAGE |
		SDHCI_QUIRK_32BIT_DMA_ADDR |
		SDHCI_QUIRK_WAIT_SEND_CMD | SDHCI_QUIRK_USE_WIDE8;
	host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;

	if (host->bus_width == 8)
		host->host_caps |= MMC_MODE_8BIT;

#if !CONFIG_IS_ENABLED(BLK)  //CONFIG_BLK
	return add_sdhci(host, 0, 400000);
#else
	return 0;
#endif
}

int s3c_sdhci_init(u32 regbase, int index, int bus_width)
{
	struct sdhci_host *host = calloc(1, sizeof(struct sdhci_host));

	if (!host) {
		printf("sdhci__host allocation fail!\n");
		return -ENOMEM;
	}
	host->ioaddr = (void *)regbase;
	host->index = index;
	host->bus_width = bus_width;

	return s3c_sdhci_core_init(host);
}

struct sdhci_host s3c64xx_sdhci_host[S3C_SDHCI_MAX_HOSTS];

int s3c_sdhc_pin_config(int peripheral, int flags)
{
	unsigned start = 0, end = 0;
	unsigned int func;

	switch (peripheral) {
	case PERIPH_ID_SDMMC0:
		start = S3C64XX_GPG(0);
		end  = S3C64XX_GPG(6);
		func = S3C_GPIO_FUNC(0x2);
		break;
	case PERIPH_ID_SDMMC1:
		start = S3C64XX_GPH(0);
		end = S3C64XX_GPH(5);
		func = S3C_GPIO_FUNC(0x2);
		break;
	case PERIPH_ID_SDMMC2:
		start = S3C64XX_GPH(6);
		end = S3C64XX_GPH(9);
		func = S3C_GPIO_FUNC(0x3);
		break;
	default:
		return -1;
	}
	for (; start <= end ; start++) {
		gpio_set_cfg(start,  func);
		gpio_set_pull(start, S3C_GPIO_PULL_NONE);
	}
	if(peripheral == PERIPH_ID_SDMMC2) {/* for cmd/clk pins*/
		gpio_set_cfg(S3C64XX_GPC(4),  3);
		gpio_set_pull(S3C64XX_GPC(4), S3C_GPIO_PULL_NONE);
		gpio_set_cfg(S3C64XX_GPC(5),  3);
		gpio_set_pull(S3C64XX_GPC(5), S3C_GPIO_PULL_NONE);
	}
	return 0;
}

#if CONFIG_IS_ENABLED(DM_MMC)

static int s3c64xx_sdhci_init(struct sdhci_host *host)
{
	int dev_id, ret;
	//flag = host->bus_width == 8 ? PINMUX_FLAG_8BIT_MODE : PINMUX_FLAG_NONE;
	switch(host->index) {
	case 2:
		dev_id = PERIPH_ID_SDMMC2; break;
	case 1:
		dev_id = PERIPH_ID_SDMMC1; break;
	case 0:
		dev_id = PERIPH_ID_SDMMC0; break;
	default:
		pr_debug("MMC: invalid device id\n");
		return -EINVAL;
	}

	s3c_sdhc_pin_config(dev_id, 0);

	if (dm_gpio_is_valid(&host->cd_gpio)) {
		ret = dm_gpio_get_value(&host->cd_gpio);
		if (!ret) {
			pr_debug("no SD card detected!\n");
			return -ENODEV;
		}
	}
	return s3c_sdhci_core_init(host);
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int s3c64xx_pinmux_decode_periph_id(const void *blob, int node)
{
	int err;
	u32 cell[3];

	err = fdtdec_get_int_array(blob, node, "interrupts", cell,
					ARRAY_SIZE(cell));
	if (err) {
		pr_debug(" invalid peripheral id\n");
		return PERIPH_ID_NONE;
	}

	return cell[1];
}

static int s3c64xx_sdhci_get_config(const void *blob, int node, struct sdhci_host *host)
{
	int bus_width, dev_id;
	unsigned int base;

	/* Get device id */
	dev_id = s3c64xx_pinmux_decode_periph_id(blob, node);
	pr_debug("%s, dev_id=%d\n", __func__, dev_id);

	switch(dev_id) {
	case PERIPH_ID_SDMMC2:
		host->index = 2; break;
	case PERIPH_ID_SDMMC1:
		host->index = 1; break;
	case PERIPH_ID_SDMMC0:
		host->index = 0; break;
	default:
		pr_debug("MMC: Can't get device id\n");
		return -EINVAL;
	}

	/* Get bus width */
	bus_width = fdtdec_get_int(blob, node, "samsung,bus-width", 0);
	if (bus_width <= 0) {
		pr_debug("MMC: Can't get bus-width\n");
		return -EINVAL;
	}
	host->bus_width = bus_width;

	/* Get the base address from the device node */
	base = fdtdec_get_addr(blob, node, "reg");
	if (!base) {
		pr_debug("MMC: Can't get base address\n");
		return -EINVAL;
	}
	host->ioaddr = (void *)base;

	//gpio_request_by_name_nodev(offset_to_ofnode(node), "pwr-gpios", 0,
	//			   &host->pwr_gpio, GPIOD_IS_OUT);
	gpio_request_by_name_nodev(offset_to_ofnode(node), "cd-gpios", 0,
				   &host->cd_gpio, GPIOD_IS_IN);

	return 0;
}
#else /* !CONFIG_IS_ENABLED(OF_CONTROL) */
static int s3c64xx_sdhci_get_config(struct sdhci_host *host)
{
	host->index = 0;
	host->bus_width = 4;
	host->ioaddr = (void *)ELFIN_HSMMC_0_BASE;
	//host->pwr_gpio.dev=NULL;
	dm_gpio_lookup_name("gpg6",&host->cd_gpio);

	return 0;
}
#endif

static void _sdhci_soft_reset (struct sdhci_host *host)
{
	unsigned int timeout;

	/*
	 * RST[0][1] : Software reset cmd/data line
	 * 1 = reset
	 * 0 = work
	 */
	sdhci_writeb(host, 0x3, SDHCI_SOFTWARE_RESET);

	host->clock = 0;

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (sdhci_readb(host,SDHCI_SOFTWARE_RESET) & 0x3) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return;
		}
		timeout--;
		udelay(1000);
	}
}

#if 0 //debug
#define s3c_readb(x)	readb((ELFIN_HSMMC_BASE) + (x))
#define s3c_readw(x)	readw((ELFIN_HSMMC_BASE) + (x))
#define s3c_readl(x)	readl((ELFIN_HSMMC_BASE) + (x))

#define print_mmc_regb(x)   \
	printf("SD_"#x": %#x\n",s3c_readb(x))

#define print_mmc_regl(x)   \
	printf("SD_"#x": %#x\n",s3c_readl(x))

#define print_mmc_regw(x)   \
	printf("SD_"#x": %#x\n",s3c_readw(x))
  
#define print_regl(x)   \
	printf(""#x": %#x\n",readl(x))

void print_reg(void)
{
  print_mmc_regb(HM_HOSTCTL);
  print_mmc_regl(HM_CONTROL2);
  print_mmc_regl(HM_CONTROL3);
  print_mmc_regw(HM_CLKCON);
  print_regl(HCLK_GATE);
  print_regl(PCLK_GATE);
  print_regl(SCLK_GATE);
  print_regl(CLK_DIV0);
  print_regl(CLK_DIV1);
  print_regl(CLK_SRC);
}
#endif
static int s3c_sdhci_reset(struct sdhci_host *host)
{
	unsigned int mask;

	_sdhci_soft_reset(host);

	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	/* mask all */
	sdhci_writel(host, 0xFFFFFFFF, SDHCI_INT_ENABLE);
	sdhci_writel(host, 0xFFFFFFFF, SDHCI_SIGNAL_ENABLE);

	sdhci_writeb(host, 0x0E, SDHCI_HOST_VERSION);	/* TMCLK * 2^27 */

	/*
	 * NORMAL Interrupt Status Enable Register init
	 * [5] ENSTABUFRDRDY : Buffer Read Ready Status Enable
	 * [4] ENSTABUFWTRDY : Buffer write Ready Status Enable
	 * [1] ENSTASTANSCMPLT : Transfre Complete Status Enable
	 * [0] ENSTACMDCMPLT : Command Complete Status Enable
	*/
	mask = (1 << 5) | (1 << 4) | (1 << 1) | (1 << 0);
	sdhci_writew(host, mask, SDHCI_INT_ENABLE);

	/*
	 * NORMAL Interrupt Signal Enable Register init
	 * [1] ENSTACMDCMPLT : Transfer Complete Signal Enable
	 */
	mask = (1 << 1);
	sdhci_writew(host, mask, SDHCI_SIGNAL_ENABLE);

	return 0;
}

static int s3c64xx_sdhci_probe(struct udevice *dev)
{
	struct s3c64xx_sdhci_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret,fin_freq;

	pr_info("dev(%s): sdhc probe ...\n",dev->name);
#if CONFIG_IS_ENABLED(OF_CONTROL)
	ret = s3c64xx_sdhci_get_config(gd->fdt_blob, dev_of_offset(dev), host);
	if (ret < 0)
		return ret;
	ret = clk_get_by_index(dev, 0, &sdhc_clk);
	if (ret < 0)
		return ret;
	fin_freq = clk_get_rate(&sdhc_clk);
	pr_info("dev(%s): source clock \"%s\",freq=%d\n",dev->name,s3c_clk_get_by_id(sdhc_clk.id)->name,fin_freq);
#else
	ret = s3c64xx_sdhci_get_config(host);
	if (ret)
		return ret;
	s3c_clk_set_parent_by_id(CLK_ID_SCLK_MMC0, CLK_ID_MOUT_EPLL);
	sdhc_clkp = s3c_clk_get_by_id(CLK_ID_SCLK_MMC0);
	if (sdhc_clkp == NULL)
		return -1;
	fin_freq = s3c_clk_get_rate(sdhc_clkp);
	pr_info("clock \"%s\": source clock freq=%d\n",sdhc_clkp->name,fin_freq);
#endif

	s3c_clk_enable_by_id(CLK_ID_HCLK_MMC0 + host->index, 1); /* clock enable for SDHCI controller */
	pr_debug("SDHCI core clock: %ld Hz\n",s3c_clk_get_rate_by_id(CLK_ID_HCLK_MMC0 + host->index));

	s3c_sdhci_reset(host);

	ret = s3c64xx_sdhci_init(host);
	if (ret)
		return ret;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 400000);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int s3c64xx_sdhci_bind(struct udevice *dev)
{
	struct s3c64xx_sdhci_plat *plat = dev_get_platdata(dev);
	int ret=0;

	pr_info("dev(%s): sdhc bind ...\n",dev->name);
	ret = sdhci_bind(dev, &plat->mmc, &plat->cfg);
	if (ret)
		pr_err("bind failed,err=%d",ret);

	return ret;
}

static const struct udevice_id s3c64xx_sdhci_ids[] = {
	{ .compatible = "samsung,s3c6410-sdhci"},
	{ }
};

U_BOOT_DRIVER(s3c64xx_sdhci_drv) = {
	.name		= "s3c64xx_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= s3c64xx_sdhci_ids,
	.bind		= s3c64xx_sdhci_bind,
	.ops		= &sdhci_ops,
	.probe		= s3c64xx_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct s3c64xx_sdhci_plat),
};
#endif /* CONFIG_DM_MMC */
