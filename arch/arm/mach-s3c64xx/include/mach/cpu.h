/*
 * (C) Copyright 2007
 * Byungjae Lee, Samsung Erectronics, bjlee@samsung.com.
 *      - only support for S3C6400
 *  $Id: s3c6410.h,v 1.8 2008/10/08 07:31:01 dark0351 Exp $
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

/************************************************
 * NAME	    : s3c6400.h
 *
 * Based on S3C6400 User's manual Rev 0.0
 ************************************************/

#ifndef __S3C6410_H__
#define __S3C6410_H__

#define S3C64XX_SPI_CHANNELS	2

#include <asm/hardware.h>

#define BIT0 			0x00000001
#define BIT1 			0x00000002
#define BIT2 			0x00000004
#define BIT3 			0x00000008
#define BIT4 			0x00000010
#define BIT5 			0x00000020
#define BIT6			0x00000040
#define BIT7			0x00000080
#define BIT8			0x00000100
#define BIT9			0x00000200
#define BIT10			0x00000400
#define BIT11			0x00000800
#define BIT12			0x00001000
#define BIT13			0x00002000
#define BIT14			0x00004000
#define BIT15			0x00008000
#define BIT16			0x00010000
#define BIT17			0x00020000
#define BIT18			0x00040000
#define BIT19			0x00080000
#define BIT20			0x00100000
#define BIT21			0x00200000
#define BIT22			0x00400000
#define BIT23			0x00800000
#define BIT24			0x01000000
#define BIT25			0x02000000
#define BIT26			0x04000000
#define BIT27			0x08000000
#define BIT28			0x10000000
#define BIT29			0x20000000
#define BIT30			0x40000000
#define BIT31			0x80000000

#define ROM_BASE0		0x00000000      /* base address of rom bank 0 */
#define ROM_BASE1		0x04000000      /* base address of rom bank 1 */
#define DRAM_BASE0		0x40000000      /* base address of dram bank 0 */
#define DRAM_BASE1		0x50000000      /* base address of dram bank 1 */

/* S3C6400 device base addresses */
#define ELFIN_LCD_BASE		0x77100000
#define ELFIN_USB_HOST_BASE	0x74300000
#define ELFIN_I2C_BASE		0x7f004000
#define ELFIN_I2S_BASE		0x7f002000
#define ELFIN_ADC_BASE		0x7e00b000
#define ELFIN_SPI_BASE		0x7f00b000

/*
 * Bus Matrix
 */
#define ELFIN_MEM_SYS_CFG	0x7e00f120

/*
 * Memory controller
 */
#define ELFIN_SROM_BASE		0x70000000

#define SROM_BW_REG		__REG(ELFIN_SROM_BASE+0x0)
#define SROM_BC0_REG		__REG(ELFIN_SROM_BASE+0x4)
#define SROM_BC1_REG		__REG(ELFIN_SROM_BASE+0x8)
#define SROM_BC2_REG		__REG(ELFIN_SROM_BASE+0xC)
#define SROM_BC3_REG		__REG(ELFIN_SROM_BASE+0x10)
#define SROM_BC4_REG		__REG(ELFIN_SROM_BASE+0x14)
#define SROM_BC5_REG		__REG(ELFIN_SROM_BASE+0x18)

/*
 * SDRAM Controller
 */
#define ELFIN_DMC0_BASE		0x7e000000
#define ELFIN_DMC1_BASE		0x7e001000

#define INDEX_DMC_MEMC_STATUS   (0x00)
#define INDEX_DMC_MEMC_CMD      (0x04)
#define INDEX_DMC_DIRECT_CMD    (0x08)
#define INDEX_DMC_MEMORY_CFG    (0x0C)
#define INDEX_DMC_REFRESH_PRD   (0x10)
#define INDEX_DMC_CAS_LATENCY   (0x14)
#define INDEX_DMC_T_DQSS        (0x18)
#define INDEX_DMC_T_MRD         (0x1C)
#define INDEX_DMC_T_RAS         (0x20)
#define INDEX_DMC_T_RC          (0x24)
#define INDEX_DMC_T_RCD         (0x28)
#define INDEX_DMC_T_RFC         (0x2C)
#define INDEX_DMC_T_RP          (0x30)
#define INDEX_DMC_T_RRD         (0x34)
#define INDEX_DMC_T_WR          (0x38)
#define INDEX_DMC_T_WTR         (0x3C)
#define INDEX_DMC_T_XP          (0x40)
#define INDEX_DMC_T_XSR         (0x44)
#define INDEX_DMC_T_ESR         (0x48)
#define INDEX_DMC_MEMORY_CFG2	(0x4C)
#define INDEX_DMC_CHIP_0_CFG    (0x200)
#define INDEX_DMC_CHIP_1_CFG    (0x204)
#define INDEX_DMC_CHIP_2_CFG    (0x208)
#define INDEX_DMC_CHIP_3_CFG    (0x20C)
#define INDEX_DMC_USER_STATUS	(0x300)
#define INDEX_DMC_USER_CONFIG	(0x304)

/*
* Memory Chip direct command
*/
#define DMC_NOP0 			0x0c0000
#define DMC_NOP1			0x1c0000
#define DMC_PA0 			0x000000	//Precharge all
#define DMC_PA1 			0x100000
#define DMC_AR0 			0x040000	//Autorefresh
#define DMC_AR1 			0x140000
#define DMC_SDR_MR0			0x080032	//MRS, CAS 3,  Burst Length 4
#define DMC_SDR_MR1			0x180032
#define DMC_DDR_MR0			0x080162
#define DMC_DDR_MR1			0x180162
#define DMC_mDDR_MR0			0x080032	//CAS 3, Burst Length 4
#define DMC_mDDR_MR1			0x180032
#define DMC_mSDR_EMR0			0x0a0000	//EMRS, DS:Full, PASR:Full Array
#define DMC_mSDR_EMR1			0x1a0000
#define DMC_DDR_EMR0			0x090000
#define DMC_DDR_EMR1			0x190000
#define DMC_mDDR_EMR0			0x0a0000	// DS:Full, PASR:Full Array
#define DMC_mDDR_EMR1			0x1a0000


/****************************************************************
 Definitions for memory configuration
 Set memory configuration
	active_chips	 = 1'b0 (1 chip)
	qos_master_chip  = 3'b000(ARID[3:0])
	memory burst	 = 3'b010(burst 4)
	stop_mem_clock	 = 1'b0(disable dynamical stop)
	auto_power_down  = 1'b0(disable auto power-down mode)
	power_down_prd	 = 6'b00_0000(0 cycle for auto power-down)
	ap_bit		 = 1'b0 (bit position of auto-precharge is 10)
	row_bits	 = 3'b010(# row address 13)
	column_bits	 = 3'b010(# column address 10 )

 Set user configuration
	2'b10=SDRAM/mSDRAM, 2'b11=DDR, 2'b01=mDDR

 Set chip select for chip [n]
	 row bank control, bank address 0x3000_0000 ~ 0x37ff_ffff
	 CHIP_[n]_CFG=0x30F8,  30: ADDR[31:24], F8: Mask[31:24]
******************************************************************/

/*
 * Interrupt
 */
#define ELFIN_VIC0_BASE_ADDR		(0x71200000)
#define ELFIN_VIC1_BASE_ADDR		(0x71300000)
#define oINTMOD				(0x0C)		// VIC INT SELECT (IRQ or FIQ)
#define oINTUNMSK			(0x10)		// VIC INT EN (Unmask by writing 1)
#define oINTMSK				(0x14)		// VIC INT EN CLEAR (Mask by writing 1)
#define oINTSUBMSK			(0x1C)		// VIC SOFT INT CLEAR
#define oVECTADDR			(0xF00)		// VIC ADDRESS


/*
 * Watchdog timer
 */
#define ELFIN_WATCHDOG_BASE		0x7E004000

#define WTCON_REG			__REG(0x7E004004)
#define WTDAT_REG			__REG(0x7E004008)
#define WTCNT_REG			__REG(0x7E00400C)

/*
 * PWM timer
 */
#define ELFIN_TIMER_BASE	0x7F006000

#define TCFG0_REG		__REG(0x7F006000)
#define TCFG1_REG		__REG(0x7F006004)
#define TCON_REG		__REG(0x7F006008)
#define TCNTB0_REG		__REG(0x7F00600c)
#define TCMPB0_REG		__REG(0x7F006010)
#define TCNTO0_REG		__REG(0x7F006014)
#define TCNTB1_REG		__REG(0x7F006018)
#define TCMPB1_REG		__REG(0x7F00601c)
#define TCNTO1_REG		__REG(0x7F006020)
#define TCNTB2_REG		__REG(0x7F006024)
#define TCMPB2_REG		__REG(0x7F006028)
#define TCNTO2_REG		__REG(0x7F00602c)
#define TCNTB3_REG		__REG(0x7F006030)
#define TCMPB3_REG		__REG(0x7F006034)
#define TCNTO3_REG		__REG(0x7F006038)
#define TCNTB4_REG		__REG(0x7F00603c)
#define TCNTO4_REG		__REG(0x7F006040)

/* Fields */
#define fTCFG0_DZONE		Fld(8,16)       /* the dead zone length (= timer 0) */
#define fTCFG0_PRE1		Fld(8,8)        /* prescaler value for time 2,3,4 */
#define fTCFG0_PRE0		Fld(8,0)        /* prescaler value for time 0,1 */
#define fTCFG1_MUX4		Fld(4,16)
/* bits */
#define TCFG0_DZONE(x)		FInsrt((x), fTCFG0_DZONE)
#define TCFG0_PRE1(x)		FInsrt((x), fTCFG0_PRE1)
#define TCFG0_PRE0(x)		FInsrt((x), fTCFG0_PRE0)
#define TCON_4_AUTO		(1 << 22)       /* auto reload on/off for Timer 4 */
#define TCON_4_UPDATE		(1 << 21)       /* manual Update TCNTB4 */
#define TCON_4_ONOFF		(1 << 20)       /* 0: Stop, 1: start Timer 4 */
#define COUNT_4_ON		(TCON_4_ONOFF*1)
#define COUNT_4_OFF		(TCON_4_ONOFF*0)
#define TCON_3_AUTO		(1 << 19)       /* auto reload on/off for Timer 3 */
#define TIMER3_ATLOAD_ON	(TCON_3_AUTO*1)
#define TIMER3_ATLAOD_OFF	FClrBit(TCON, TCON_3_AUTO)
#define TCON_3_INVERT		(1 << 18)       /* 1: Inverter on for TOUT3 */
#define TIMER3_IVT_ON		(TCON_3_INVERT*1)
#define TIMER3_IVT_OFF		(FClrBit(TCON, TCON_3_INVERT))
#define TCON_3_MAN		(1 << 17)       /* manual Update TCNTB3,TCMPB3 */
#define TIMER3_MANUP		(TCON_3_MAN*1)
#define TIMER3_NOP		(FClrBit(TCON, TCON_3_MAN))
#define TCON_3_ONOFF		(1 << 16)       /* 0: Stop, 1: start Timer 3 */
#define TIMER3_ON		(TCON_3_ONOFF*1)
#define TIMER3_OFF		(FClrBit(TCON, TCON_3_ONOFF))
/* macros */
#define GET_PRESCALE_TIMER4(x)	FExtr((x), fTCFG0_PRE1)
#define GET_DIVIDER_TIMER4(x)	FExtr((x), fTCFG1_MUX4)

/*
 * RTC Controller
 */
#define ELFIN_RTC_BASE		0x7e005000

#define RTCCON_REG		__REG(0x7e005040)
#define TICNT_REG		__REG(0x7e005044)
#define RTCALM_REG		__REG(0x7e005050)
#define ALMSEC_REG		__REG(0x7e005054)
#define ALMMIN_REG		__REG(0x7e005058)
#define ALMHOUR_REG		__REG(0x7e00505c)
#define ALMDATE_REG		__REG(0x7e005060)
#define ALMMON_REG		__REG(0x7e005064)
#define ALMYEAR_REG		__REG(0x7e005068)
#define BCDSEC_REG		__REG(0x7e005070)
#define BCDMIN_REG		__REG(0x7e005074)
#define BCDHOUR_REG		__REG(0x7e005078)
#define BCDDATE_REG		__REG(0x7e00507c)
#define BCDDAY_REG		__REG(0x7e005080)
#define BCDMON_REG		__REG(0x7e005084)
#define BCDYEAR_REG		__REG(0x7e005088)

/*
 * USB2.0 HS OTG (Chapter 26)
 */
#define USBOTG_LINK_BASE	(0x7C000000)
#define USBOTG_PHY_BASE		(0x7C100000)

/* Core Global Registers */
#define S3C_OTG_GOTGCTL		(USBOTG_LINK_BASE + 0x000)	/* OTG Control & Status */
#define S3C_OTG_GOTGINT		(USBOTG_LINK_BASE + 0x004)	/* OTG Interrupt */
#define S3C_OTG_GAHBCFG		(USBOTG_LINK_BASE + 0x008)	/* Core AHB Configuration */
#define S3C_OTG_GUSBCFG		(USBOTG_LINK_BASE + 0x00C)	/* Core USB Configuration */
#define S3C_OTG_GRSTCTL		(USBOTG_LINK_BASE + 0x010)	/* Core Reset */
#define S3C_OTG_GINTSTS		(USBOTG_LINK_BASE + 0x014)	/* Core Interrupt */
#define S3C_OTG_GINTMSK		(USBOTG_LINK_BASE + 0x018)	/* Core Interrupt Mask */
#define S3C_OTG_GRXSTSR		(USBOTG_LINK_BASE + 0x01C)	/* Receive Status Debug Read/Status Read */
#define S3C_OTG_GRXSTSP		(USBOTG_LINK_BASE + 0x020)	/* Receive Status Debug Pop/Status Pop */
#define S3C_OTG_GRXFSIZ		(USBOTG_LINK_BASE + 0x024)	/* Receive FIFO Size */
#define S3C_OTG_GNPTXFSIZ	(USBOTG_LINK_BASE + 0x028)	/* Non-Periodic Transmit FIFO Size */
#define S3C_OTG_GNPTXSTS	(USBOTG_LINK_BASE + 0x02C)	/* Non-Periodic Transmit FIFO/Queue Status */

#define S3C_OTG_HPTXFSIZ	(USBOTG_LINK_BASE + 0x100)	/* Host Periodic Transmit FIFO Size */
#define S3C_OTG_DPTXFSIZ1	(USBOTG_LINK_BASE + 0x104)	/* Device Periodic Transmit FIFO-1 Size */
#define S3C_OTG_DPTXFSIZ2	(USBOTG_LINK_BASE + 0x108)	/* Device Periodic Transmit FIFO-2 Size */
#define S3C_OTG_DPTXFSIZ3	(USBOTG_LINK_BASE + 0x10C)	/* Device Periodic Transmit FIFO-3 Size */
#define S3C_OTG_DPTXFSIZ4	(USBOTG_LINK_BASE + 0x110)	/* Device Periodic Transmit FIFO-4 Size */
#define S3C_OTG_DPTXFSIZ5	(USBOTG_LINK_BASE + 0x114)	/* Device Periodic Transmit FIFO-5 Size */
#define S3C_OTG_DPTXFSIZ6	(USBOTG_LINK_BASE + 0x118)	/* Device Periodic Transmit FIFO-6 Size */
#define S3C_OTG_DPTXFSIZ7	(USBOTG_LINK_BASE + 0x11C)	/* Device Periodic Transmit FIFO-7 Size */
#define S3C_OTG_DPTXFSIZ8	(USBOTG_LINK_BASE + 0x120)	/* Device Periodic Transmit FIFO-8 Size */
#define S3C_OTG_DPTXFSIZ9	(USBOTG_LINK_BASE + 0x124)	/* Device Periodic Transmit FIFO-9 Size */
#define S3C_OTG_DPTXFSIZ10	(USBOTG_LINK_BASE + 0x128)	/* Device Periodic Transmit FIFO-10 Size */
#define S3C_OTG_DPTXFSIZ11	(USBOTG_LINK_BASE + 0x12C)	/* Device Periodic Transmit FIFO-11 Size */
#define S3C_OTG_DPTXFSIZ12	(USBOTG_LINK_BASE + 0x130)	/* Device Periodic Transmit FIFO-12 Size */
#define S3C_OTG_DPTXFSIZ13	(USBOTG_LINK_BASE + 0x134)	/* Device Periodic Transmit FIFO-13 Size */
#define S3C_OTG_DPTXFSIZ14	(USBOTG_LINK_BASE + 0x138)	/* Device Periodic Transmit FIFO-14 Size */
#define S3C_OTG_DPTXFSIZ15	(USBOTG_LINK_BASE + 0x13C)	/* Device Periodic Transmit FIFO-15 Size */
	
/* Host Global Registers */
#define S3C_OTG_HCFG		(USBOTG_LINK_BASE + 0x400)	/* Host Configuration */
#define S3C_OTG_HFIR		(USBOTG_LINK_BASE + 0x404)	/* Host Frame Interval */
#define S3C_OTG_HFNUM		(USBOTG_LINK_BASE + 0x408)	/* Host Frame Number/Frame Time Remaining */
#define S3C_OTG_HPTXSTS		(USBOTG_LINK_BASE + 0x410)	/* Host Periodic Transmit FIFO/Queue Status */
#define S3C_OTG_HAINT		(USBOTG_LINK_BASE + 0x414)	/* Host All Channels Interrupt */
#define S3C_OTG_HAINTMSK	(USBOTG_LINK_BASE + 0x418)	/* Host All Channels Interrupt Mask */

/* Host Port Control & Status Registers */
#define S3C_OTG_HPRT		(USBOTG_LINK_BASE + 0x440)	/* Host Port Control & Status */

/* Host Channel-Specific Registers */
#define S3C_OTG_HCCHAR0		(USBOTG_LINK_BASE + 0x500)	/* Host Channel-0 Characteristics */
#define S3C_OTG_HCSPLT0		(USBOTG_LINK_BASE + 0x504)	/* Host Channel-0 Split Control */
#define S3C_OTG_HCINT0		(USBOTG_LINK_BASE + 0x508)	/* Host Channel-0 Interrupt */
#define S3C_OTG_HCINTMSK0	(USBOTG_LINK_BASE + 0x50C)	/* Host Channel-0 Interrupt Mask */
#define S3C_OTG_HCTSIZ0		(USBOTG_LINK_BASE + 0x510)	/* Host Channel-0 Transfer Size */
#define S3C_OTG_HCDMA0		(USBOTG_LINK_BASE + 0x514)	/* Host Channel-0 DMA Address */

	
/* Device Global Registers */
#define S3C_OTG_DCFG		(USBOTG_LINK_BASE + 0x800)	/* Device Configuration */
#define S3C_OTG_DCTL		(USBOTG_LINK_BASE + 0x804)	/* Device Control */
#define S3C_OTG_DSTS		(USBOTG_LINK_BASE + 0x808)	/* Device Status */
#define S3C_OTG_DIEPMSK 	(USBOTG_LINK_BASE + 0x810)	/* Device IN Endpoint Common Interrupt Mask */
#define S3C_OTG_DOEPMSK 	(USBOTG_LINK_BASE + 0x814)	/* Device OUT Endpoint Common Interrupt Mask */
#define S3C_OTG_DAINT		(USBOTG_LINK_BASE + 0x818)	/* Device All Endpoints Interrupt */
#define S3C_OTG_DAINTMSK	(USBOTG_LINK_BASE + 0x81C)	/* Device All Endpoints Interrupt Mask */
#define S3C_OTG_DTKNQR1 	(USBOTG_LINK_BASE + 0x820)	/* Device IN Token Sequence Learning Queue Read 1 */
#define S3C_OTG_DTKNQR2 	(USBOTG_LINK_BASE + 0x824)	/* Device IN Token Sequence Learning Queue Read 2 */
#define S3C_OTG_DVBUSDIS	(USBOTG_LINK_BASE + 0x828)	/* Device VBUS Discharge Time */
#define S3C_OTG_DVBUSPULSE	(USBOTG_LINK_BASE + 0x82C)	/* Device VBUS Pulsing Time */
#define S3C_OTG_DTKNQR3 	(USBOTG_LINK_BASE + 0x830)	/* Device IN Token Sequence Learning Queue Read 3 */
#define S3C_OTG_DTKNQR4 	(USBOTG_LINK_BASE + 0x834)	/* Device IN Token Sequence Learning Queue Read 4 */
	
/* Device Logical IN Endpoint-Specific Registers */
#define S3C_OTG_DIEPCTL0	(USBOTG_LINK_BASE + 0x900)	/* Device IN Endpoint 0 Control */
#define S3C_OTG_DIEPINT0	(USBOTG_LINK_BASE + 0x908)	/* Device IN Endpoint 0 Interrupt */
#define S3C_OTG_DIEPTSIZ0	(USBOTG_LINK_BASE + 0x910)	/* Device IN Endpoint 0 Transfer Size */
#define S3C_OTG_DIEPDMA0	(USBOTG_LINK_BASE + 0x914)	/* Device IN Endpoint 0 DMA Address */

/* Device Logical OUT Endpoint-Specific Registers */
#define S3C_OTG_DOEPCTL0	(USBOTG_LINK_BASE + 0xB00)	/* Device OUT Endpoint 0 Control */
#define S3C_OTG_DOEPINT0	(USBOTG_LINK_BASE + 0xB08)	/* Device OUT Endpoint 0 Interrupt */
#define S3C_OTG_DOEPTSIZ0	(USBOTG_LINK_BASE + 0xB10)	/* Device OUT Endpoint 0 Transfer Size */
#define S3C_OTG_DOEPDMA0	(USBOTG_LINK_BASE + 0xB14)	/* Device OUT Endpoint 0 DMA Address */
	
/* Power & clock gating registers */
#define S3C_OTG_PCGCCTRL	(USBOTG_LINK_BASE + 0xE00)

/* Endpoint FIFO address */
#define S3C_OTG_EP0_FIFO	(USBOTG_LINK_BASE + 0x1000)

 

/* OTG PHY CORE REGISTERS */
#define S3C_OTG_PHYPWR		(USBOTG_PHY_BASE+0x00)
#define S3C_OTG_PHYCTRL		(USBOTG_PHY_BASE+0x04)
#define S3C_OTG_RSTCON		(USBOTG_PHY_BASE+0x08)

/*
 * Modem interface
 */

#define ELFIN_MODEM_BASE    0x74108000

/* include common stuff */

/* PENDING BIT */
#define BIT_EINT0		(0x1)
#define BIT_EINT1		(0x1<<1)
#define BIT_EINT2		(0x1<<2)
#define BIT_EINT3		(0x1<<3)
#define BIT_EINT4_7		(0x1<<4)
#define BIT_EINT8_23		(0x1<<5)
#define BIT_BAT_FLT		(0x1<<7)
#define BIT_TICK		(0x1<<8)
#define BIT_WDT			(0x1<<9)
#define BIT_TIMER0		(0x1<<10)
#define BIT_TIMER1		(0x1<<11)
#define BIT_TIMER2		(0x1<<12)
#define BIT_TIMER3		(0x1<<13)
#define BIT_TIMER4		(0x1<<14)
#define BIT_UART2		(0x1<<15)
#define BIT_LCD			(0x1<<16)
#define BIT_DMA0		(0x1<<17)
#define BIT_DMA1		(0x1<<18)
#define BIT_DMA2		(0x1<<19)
#define BIT_DMA3		(0x1<<20)
#define BIT_SDI			(0x1<<21)
#define BIT_SPI0		(0x1<<22)
#define BIT_UART1		(0x1<<23)
#define BIT_USBH		(0x1<<26)
#define BIT_IIC			(0x1<<27)
#define BIT_UART0		(0x1<<28)
#define BIT_SPI1		(0x1<<29)
#define BIT_RTC			(0x1<<30)
#define BIT_ADC			(0x1<<31)
#define BIT_ALLMSK		(0xFFFFFFFF)

#ifndef __ASSEMBLY__
#include "s3c64x0.h"
#endif

#endif /*__S3C6410_H__*/
