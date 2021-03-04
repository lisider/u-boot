#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_MMC_BOOT)
#include <common.h>
#include <asm/io.h>
#include <asm/arch/sdhc.h>
#include <asm/arch/movi.h>

int mmc_bl2_copy(uint *bl2_base)
{
	int ret;

	/* Set current driver to  9mA */
	writel(readl(HM_CONTROL4) | (0x3 << 16), HM_CONTROL4);
	ret = CopyMovitoMem(HSMMC_CHANNEL, MOVI_BL2_POS, MOVI_BL2_BLKCNT,
						bl2_base, MOVI_INIT_REQUIRED);
	return ret;
}

void spl_boot_mmc(void)
{
	ulong bl2base=BL2_BASE;

	if(movi_sdmmc_is_detected) {
		mmc_bl2_copy((uint *)bl2base);
		((void (*)(void))bl2base)();
	}
}

// for debug
#if  0 //DEBUG
extern void asm_puts(char *s);
extern void asm_print_hex(ulong val);
#define s3c_readw(x)	readw((ELFIN_HSMMC_BASE + (HSMMC_CHANNEL * 0x100000)) + (x))
#define s3c_readl(x)	readl((ELFIN_HSMMC_BASE + (HSMMC_CHANNEL * 0x100000)) + (x))

#define print_mmc_regl(x)   \
	{ asm_puts("\n"#x": "); \
	  asm_print_hex(s3c_readl(x));}

#define print_mmc_regw(x)   \
	{ asm_puts("\n"#x": "); \
	  asm_print_hex(s3c_readw(x));}
  
#define print_regl(x)   \
	{ asm_puts("\n"#x": "); \
	  asm_print_hex(readl(x));}

void spl_print_test(void)
{
	ulong tmp,clkdiv0=CLK_DIV0_REG;
	asm_puts("\nsdcard total sectors: ");
	asm_print_hex(MOVI_TOTAL_BLKCNT);
	asm_puts("\nsdcard capacity: ");
	asm_print_hex(MOVI_HIGH_CAPACITY);
	asm_puts("\nclkdiv0: ");
	asm_print_hex(clkdiv0);
	asm_puts("\nMCLK: ");tmp=spl_uart_getclk(1);
	if(clkdiv0 & S3C64XX_CLKDIV0_MPLL_MASK)
		tmp <<=1;
	asm_print_hex(tmp);
	asm_puts("\nECLK: ");
	asm_print_hex(spl_uart_getclk(0));
	if(OTHERS_REG & 0x40) { // SYNC MUX
		ulong r, m, p, s;

		r = APLL_CON_REG;
		m = (r>>16) & 0x3ff;
		p = (r>>8) & 0x3f;
		s = r & 0x7;

		tmp= (m * (CONFIG_SYS_CLK_FREQ / (p * (1 << s)))); 
		asm_puts("\nACLK: ");
		asm_print_hex(tmp);

		asm_puts("\nHCLK from sync!");
	}
	{
		uint hclkx2_div = ((CLK_DIV0_REG>>9) & 0x7) + 1;
		uint _div;

		asm_puts("\nHCLKx2: ");tmp=tmp /hclkx2_div;
		asm_print_hex(tmp);
		asm_puts("\nHCLK: ");
		_div = ((clkdiv0>>8) & 0x1) + 1;
		asm_print_hex(tmp/_div);asm_puts(" _div: ");
		asm_print_hex(_div);
		_div = ((clkdiv0>>12) & 0xf) + 1;
		asm_puts("\nPCLK: ");
		asm_print_hex(tmp/_div);asm_puts(" _div: ");
		asm_print_hex(_div);
	}
	asm_puts("\n\nSDHC Reg to show");
	print_mmc_regl(HM_CONTROL2);
	print_mmc_regl(HM_CONTROL3);
	print_mmc_regw(HM_CLKCON);
	print_regl(HCLK_GATE);
	print_regl(PCLK_GATE);
	print_regl(SCLK_GATE);
	print_regl(MEM0_CLK_GATE);
	print_regl(CLK_DIV0);
	print_regl(CLK_DIV1);
	print_regl(CLK_SRC);
	asm_puts("\nclear sclk mmc0_48,but gate sclk_mmc0");
	asm_puts("\ncan't clear hclk mmc0");
	SCLK_GATE_REG &=  ~ (0x8 << 24); //clear MMC0_48 ,but gate MMC0
	HCLK_GATE_REG |= S3C_CLKCON_HCLK_HSMMC0;
	print_regl(SCLK_GATE);
	asm_puts("\n\n");
}
#endif //DEBUG

#endif // CONFIG_SPL_BUILD  && CONFIG_SPL_MMC_BOOT

