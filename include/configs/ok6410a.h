/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * HeungJun Kim <riverful.kim@samsung.com>
 * Inki Dae <inki.dae@samsung.com>
 *
 * Configuation settings for the SAMSUNG SMDKC100 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/mach-types.h>

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000	/* Forlink OK6410 has 12MHz input clock */

/* DRAM Base */
#define PHYS_SDRAM_1		0x50000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x10000000 /* 256 MB */

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define SDRAM_BANK_SIZE		(PHYS_SDRAM_1_SIZE)

/* total memory required by uboot */
#define CFG_UBOOT_RAM_SIZE		(0x10000000 - (CONFIG_SYS_TEXT_BASE&0x0FFFFFFF))  /*last 5 MB */
#define CFG_UBOOT_TEXT_LIMIT  (512<<10)  /* 512 KB */
/* base address for uboot */
#define CONFIG_SYS_PHY_UBOOT_BASE	  \
	( CONFIG_SYS_SDRAM_BASE + SDRAM_BANK_SIZE - CFG_UBOOT_RAM_SIZE )
#define CONFIG_SYS_UBOOT_BASE	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_UBOOT_BASE - 0x0C)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE	/* default load address	*/

#define CFG_PHY_SRAM_BASE 0x0C000000
#define CFG_PHY_SRAM_SIZE 0x00002000
/*
 * SPL 
 */
//#ifdef CONFIG_SPL
//#define CONFIG_SPL_TEXT_BASE 0x0C000000  /* CFG_PHY_SRAM_BASE */
///* in s3c6410, the spl base seems to be flowed ,need to relocat, refer to fact value*/
///* #define CFG_RELOCATE_SPL  */
//#endif

/*
 * Boot configuration
 */
//#define CONFIG_ENV_SIZE			(16 << 10)	/* Total Size of Environment Sector */
//#if defined(CONFIG_SPL_NAND_BOOT)
//#define CONFIG_ENV_ADDR			(512 << 10)	/* 512, 0x80000 */
////#define CONFIG_ENV_OFFSET			(512 << 10)	
//#else /* CONFIG_SPL_MMC_BOOT */
//#define CONFIG_ENV_ADDR			(0)	/* 256KiB, 0x40000 */
////#define CONFIG_ENV_OFFSET			(0-(25 << 10))	/* -25KiB ,see dts: u-boot,mmc-env-offset */
//#endif

/*
 * Size of malloc() pool
 */
#ifndef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN (1024*1024)
#endif 

/* Text Base */

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG

#define BOARD_LATE_INIT

/*
 * Architecture magic and machine type
 */
#define CONFIG_MACH_TYPE	MACH_TYPE_SMDK6410
#define UBOOT_MAGIC		(0x43090000 | MACH_TYPE)

/* Power Management is enabled */
#define CONFIG_PM

 /*
 * LCD configs
 */
#ifdef CONFIG_S3C_LCD_LOGO
#undef CONFIG_LCD_LOGO   /* don't used when S3C_LCD_LOGO_IMAGE */
#endif

/*
 * Hardware drivers
 */
#define CFG_HARD_I2C		1
#define CFG_I2C_SPEED		10000
#define CFG_I2C_SLAVE		0xFE

/*
 * usb driver
 */
#define CONFIG_SYS_USB_OHCI_REGS_BASE ELFIN_USB_HOST_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"s3c6410-ohci"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	1
#define CONFIG_SYS_USB_OHCI_CPU_INIT

#ifdef CONFIG_USB_S3C64XX
#define CONFIG_USB_OHCI_NEW
#endif 

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#define CONFIG_DRIVER_DM9000
#define CONFIG_DM9000_BASE (0x18000300) /*XM0CSN1*/
#define DM9000_IO (CONFIG_DM9000_BASE)
#define DM9000_DATA (CONFIG_DM9000_BASE+0x4) /*ADDR2*/

#define CONFIG_ETHADDR		00:40:5c:26:0a:5b
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.1.20
#define CONFIG_SERVERIP		192.168.1.10
#define CONFIG_GATEWAYIP	192.168.1.1

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory		*/
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE		384		/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + PHYS_SDRAM_1_SIZE - CFG_UBOOT_SIZE)		/* 256 MB in DRAM	*/

#undef CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */


/* the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need */
/* it to wrap 100 times (total 1562500) to get 1 sec. */
#define CFG_HZ			1562500		/* at PCLK 50MHz  */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* 256 KiB */

#if !defined(CONFIG_NAND_SPL) && (CONFIG_SYS_TEXT_BASE >= 0xC0000000)
#define CONFIG_ENABLE_MMU
#endif

#ifdef CONFIG_ENABLE_MMU
#define CONFIG_SYS_MAPPED_RAM_BASE	0xC0000000
#else
#define CONFIG_SYS_MAPPED_RAM_BASE	CONFIG_SYS_SDRAM_BASE
#endif

/*
 *  PLL/Clock configs
 */

/* select one of define :  */
#define   CFG_CLK_A800_M532_E96 0x0001
#define   CFG_CLK_A666_M532_E96 0x0002
#define   CFG_CLK_A532_M532_E96 0x0003
#define   CFG_CLK_A400_M532_E96 0x0004
#define   CFG_CLK_OTHERS             0x0006

#define CFG_CLK_SET CFG_CLK_A532_M532_E96
#undef CFG_CLKSRC_CLKUART  /*   EXT_UCLK1  by dividing MPLL output (fixed in lowlevel_init.S ) */
                            /* or PCLK will be selected if undefined */
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_FLASH_BASE		0x00000000
#define CFG_MAX_FLASH_BANKS	0	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	1024
#define CFG_AMD_LV800
#define PHYS_FLASH_SIZE		0x100000

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

/* NAND configuration */
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#define CONFIG_SYS_NAND_BASE           (0x70200010)
#define CONFIG_SYS_NAND_MAX_CHIPS      1

/*
 * ENV setting in MMC device
 */
#ifdef CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 0
#endif

#ifdef CONFIG_SPL_MMC_BOOT
#define CONFIG_BOOTCOMMAND	"fatload mmc 0:1 0x50008000 zImage;bootm 0x50008000"
#else
/* Settings as above boot configuration */
#define CONFIG_BOOTCOMMAND	"nand read 0x50008000 0x300000 0x180000;bootm 0x50008000"
#endif 

/* early uart debug */
#undef CONFIG_DEBUG_UART_BASE
#define CONFIG_DEBUG_UART_BASE  0x7F005000
/* #define CONFIG_BAUDRATE  115200  same as default*/
#undef CONFIG_DEBUG_UART_CLOCK
#define CONFIG_DEBUG_UART_CLOCK   get_uart_clk(0)


#endif	/* __CONFIG_H */
