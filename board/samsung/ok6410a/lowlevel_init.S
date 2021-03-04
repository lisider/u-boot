/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *                     Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the Samsung SMDK2410 by
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
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


#include <asm/arch/config.h>
#include <version.h>

#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock-regs.h>
#include <asm/arch/uart.h>
#include <asm/arch/nand-regs.h>
#include <asm/arch/gpio-regs.h>
#include "ok6410a_val.h"

#ifdef CONFIG_SPL_BUILD 
#define CFG_CLKSRC_CLKUART
#endif

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

	.globl	_UBRDIV
_UBRDIV:
#if defined(CFG_UART_50)
	.word	0x1A
#elif defined(CFG_UART_66)
	.word	0x1C
	//.word	0x23
#else
	.word	0x23
#endif

	.globl	_UDIVSLOT
_UDIVSLOT:
#if defined(CFG_UART_50)
	.word	0x0808
#elif defined(CFG_UART_66)
	.word	0xDFDD
	//.word	0x0080
#else
	.word	0x3
#endif

	.globl lowlevel_init
lowlevel_init:
	mov	r12, lr

	ldr	r0, =ELFIN_GPIO_BASE
	ldr     r1, =0x55555555	
	str     r1, [r0, #GPKCON0_OFFSET]

	ldr     r1, =0x55555555
	str     r1, [r0, #GPKCON1_OFFSET]

	ldr     r1, =0x22222666
	str     r1, [r0, #GPLCON0_OFFSET]

    /* close lcd backlight after power up */
	mov r0, #0  /* 0 - off ; others - on */
	bl  lcd_backlight_switch
	
 	/* LED on only #8 */
	ldr	r0, =ELFIN_GPIO_BASE  
	ldr	r1, =0x00111111
	str	r1, [r0, #GPMCON_OFFSET]

	ldr	r1, =0x00000555
	str	r1, [r0, #GPMPUD_OFFSET]

	ldr	r1, =0x002a
	str	r1, [r0, #GPMDAT_OFFSET]

	ldr	r1, =0  /*0x55555555 phantom*/
	str	r1, [r0, #MEM1DRVCON_OFFSET]

	/* Disable Watchdog */
	ldr	r0, =0x7e000000		@0x7e004000
	orr	r0, r0, #0x4000
	mov	r1, #0
	str	r1, [r0]

	/* External interrupt pending clear */
	ldr	r0, =(ELFIN_GPIO_BASE+EINTPEND_OFFSET)	/*EINTPEND*/
	ldr	r1, [r0]
	str	r1, [r0]

	ldr	r0, =ELFIN_VIC0_BASE_ADDR	@0x71200000
	ldr	r1, =ELFIN_VIC1_BASE_ADDR	@0x71300000

	/* Disable all interrupts (VIC0 and VIC1) */
	mvn	r3, #0x0
	str	r3, [r0, #oINTMSK]
	str	r3, [r1, #oINTMSK]

	/* Set all interrupts as IRQ */
	mov	r3, #0x0
	str	r3, [r0, #oINTMOD]
	str	r3, [r1, #oINTMOD]

	/* Pending Interrupt Clear */
	mov	r3, #0x0
	str	r3, [r0, #oVECTADDR]
	str	r3, [r1, #oVECTADDR]

#ifdef CONFIG_SPL_BUILD 
/* use default bl0 clock-setting in SPL 
 * APLL : 220,3,2    ---220MHz if FIN=12MHz
 * (ACLK:183.3~366.7Mhz)
 * MPLL : 220,3,3    ---110MHz if FIN=12MHz
 * (HCLK:45.8~91.7Mhz)
 * EPLL : 60,2,4,0   ---22.5MHz if FIN=12MHz
 * (MMC CLK:18.75~37.5Mhz
 *//* select MPLL  as uart source */
	ldr	r0, =ELFIN_CLOCK_POWER_BASE /* 0x7e00f000 */
	ldr	r1, [r0, #CLK_SRC_OFFSET]
#if defined(CFG_CLKSRC_CLKUART)
	orr	r1, r1, #0x2000   /*bit[13], 0:MoutEPLL, 1: DoutMPLL */
#else
	bic   r1, r1, #0x2000
#endif
	str	r1, [r0, #CLK_SRC_OFFSET]

	/* ldr	sp, =CONFIG_SYS_PHY_UBOOT_BASE // don't do it ,because DDR is not ready now*/ 
#if defined(CFG_CLKSRC_CLKUART) 
	mov r0, # 1 /* uart clk in, 0: EPLL  1: MPLL */
#else
	mov r0, # 0 /* uart clk in, 0: EPLL  1: MPLL */
#endif
	bl spl_config_uart_baudrate
#else /* ! CONFIG_SPL_BUILD  */
	/* init system clock */
	bl system_clock_init
#endif

	/* for UART */
	bl uart_asm_init

	/* uart print char*/
	mov	r0, #0x4F @'O'
	bl asm_putc 
	mov	r0, #0x4B @'K'
	bl asm_putc 

#if defined(CONFIG_NAND)
	/* simple init for NAND */
	bl nand_asm_init
#endif

#if 0
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp 	r1, r2			/* compare r0, r1                  */
	beq	1f			/* r0 == r1 then skip sdram init   */
#endif
	mov r0, #0x2B  @'+'
	bl    asm_putc

	/* Memory subsystem address 0x7e00f120 */
	ldr r0, =ELFIN_MEM_SYS_CFG

	/* Xm0CSn2 = NFCON CS0, Xm0CSn3 = NFCON CS1 */
	mov	r1, #0x0D
	str	r1, [r0]

	mov r0, #0x2B  @'+'
	bl    asm_putc
	
	bl	mem_ctrl_asm_init

	mov r0, #0x2D  @'-'
	bl    asm_putc
	mov r0, #0x2D  @'-'
	bl    asm_putc

/* Wakeup support. Don't know if it's going to be used, untested. */
	ldr     r0, =(ELFIN_CLOCK_POWER_BASE + RST_STAT_OFFSET)
	ldr     r1, [r0]
	bic     r1, r1, #0xfffffff7
	cmp     r1, #0x8
	beq     wakeup_reset

1:
	mov	lr, r12
	mov	pc, lr

wakeup_reset:
/*	
	ldr   r0, =(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
	ldr	r0,[r0]
	bl asm_print_hex
*/
	mov   r0, #0x2D  @'-'
	bl    asm_putc
	
	/* Clear wakeup status register */
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE + WAKEUP_STAT_OFFSET)
	ldr	r1, [r0]
	str	r1, [r0]

	/* LED test */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x3
	str	r1, [r0, #GPMDAT_OFFSET]

	/* Load return address and jump to kernel */
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE + INF_REG0_OFFSET)
	/* r1 = physical address of s3c6400_cpu_resume function */
	ldr	r1, [r0]
	/* Jump to kernel (sleep-s3c6400.S) */
	mov	pc, r1
	nop
	nop
/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:
	ldr	r0, =ELFIN_CLOCK_POWER_BASE	/* 0x7e00f000 */

#ifdef	CFG_SYNC_MODE
	ldr	r1, [r0, #OTHERS_OFFSET]
	mov	r2, #0x40
	orr	r1, r1, r2
	str	r1, [r0, #OTHERS_OFFSET]

	nop
	nop
	nop
	nop
	nop

	ldr	r2, =0x80
	orr	r1, r1, r2
	str	r1, [r0, #OTHERS_OFFSET]

check_syncack:
	ldr	r1, [r0, #OTHERS_OFFSET]
	and	r1, r1, #0xf00
	cmp	r1, #0xf00
	bne	check_syncack
#else	/* ASYNC Mode */
	nop
	nop
	nop
	nop
	nop

	/*
	 * This was unconditional in original Samsung sources, but it doesn't
	 * seem to make much sense on S3C6400.
	 */
	ldr	r1, [r0, #OTHERS_OFFSET]
	bic	r1, r1, #0xC0
	orr	r1, r1, #0x40
	str	r1, [r0, #OTHERS_OFFSET]

wait_for_async:
	ldr	r1, [r0, #OTHERS_OFFSET]
	and	r1, r1, #0xf00
	cmp	r1, #0x0
	bne	wait_for_async

	ldr	r1, [r0, #OTHERS_OFFSET]
	bic	r1, r1, #0x40
	str	r1, [r0, #OTHERS_OFFSET]
#endif

	/*Required period to generate a stable clock output*/
	mov	r1, #0xff00
	orr	r1, r1, #0xff
	str	r1, [r0, #APLL_LOCK_OFFSET]
	str	r1, [r0, #MPLL_LOCK_OFFSET]
	str	r1, [r0, #EPLL_LOCK_OFFSET]


#if defined(CFG_CLKSRC_CLKUART)
/* CLKUART(=66.5Mhz) = CLKUART_input(532/2=266Mhz) / (UART_RATIO(3)+1) */
/* CLKUART(=50Mhz) = CLKUART_input(400/2=200Mhz) / (UART_RATIO(3)+1) */
/* Now, When you use UART CLK SRC by EXT_UCLK1, We support 532MHz & 400MHz value */
	ldr   	r1, [r0, #CLK_DIV2_OFFSET]
	bic	r1, r1, #0xF0000
	orr	r1, r1, #(3<<16)
	str	r1, [r0, #CLK_DIV2_OFFSET]
#endif

	/* Set Clock Divider */
	ldr	r1, [r0, #CLK_DIV0_OFFSET]
	bic	r1, r1, #0xff00
	bic	r1, r1, #0xff
	ldr	r2, =CLK_DIV_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]

	ldr	r1, =APLL_VAL
	str	r1, [r0, #APLL_CON_OFFSET]
	ldr	r1, =MPLL_VAL
	str	r1, [r0, #MPLL_CON_OFFSET]

	/* FOUT of EPLL is 96MHz */
	ldr	r1, =EPLL_VAL0
	str	r1, [r0, #EPLL_CON0_OFFSET]
	ldr	r1, =EPLL_VAL1
	str	r1, [r0, #EPLL_CON1_OFFSET]

	/* APLL, MPLL, EPLL select to Fout */
	ldr	r1, [r0, #CLK_SRC_OFFSET]
	mov	r2, #0x7
#if defined(CFG_CLKSRC_CLKUART)
	orr	r2, r2, #0x2000   /*bit[13], 0:MoutEPLL, 1: DoutMPLL */
#else
	bic   r2, r2, #0x2000
#endif
	orr	r1, r1, r2

	str	r1, [r0, #CLK_SRC_OFFSET]

	/* wait at least 200us to stablize all clock */
	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	/* Synchronization for VIC port */
#if 0 /* defined(CFG_SYNC_MODE) --not need in S3c6410 */
	ldr	r1, [r0, #OTHERS_OFFSET]
	orr	r1, r1, #0x20
	str	r1, [r0, #OTHERS_OFFSET]
#endif
	mov	pc, lr

/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:
	/* set GPIO to enable UART */
	@ GPIO setting for UART
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x220022
	str   	r1, [r0, #GPACON_OFFSET]

	ldr	r1, =0x2222
	str   	r1, [r0, #GPBCON_OFFSET]

	ldr	r0, =ELFIN_UART_CONSOLE_BASE		@0x7F005000
	mov	r1, #0x0
	str	r1, [r0, #UFCON_OFFSET]
	str	r1, [r0, #UMCON_OFFSET]

	mov	r1, #0x3                	@was 0.
	str	r1, [r0, #ULCON_OFFSET]

#if defined(CFG_CLKSRC_CLKUART)
	ldr	r1, =0xe45			/* UARTCLK SRC = 11 => EXT_UCLK1*/
#else
	ldr	r1, [r0, #PCLK_GATE_OFFSET]  
	orr	r1, r1, # 0x02    /* Gate  for PCLK_UART0*/
	str   r1, [r0, #PCLK_GATE_OFFSET] 
	ldr	r1, =0xA45			/* UARTCLK SRC = x0 => PCLK */
#endif

	str	r1, [r0, #UCON_OFFSET]

	adr	r1, _UBRDIV
	ldr	r1, [r1]
	str	r1, [r0, #UBRDIV_OFFSET]

	adr	r1, _UDIVSLOT
	ldr	r1, [r1]
	str	r1, [r0, #UDIVSLOT_OFFSET]
	mov	pc, lr

asm_putc:
	ldr	r1, =ELFIN_UART_BASE
1:	ldr   r2, [r1, #UTRSTAT_OFFSET]
	tst   r2, #2
	beq  1b
	str	r0, [r1, #UTXH_OFFSET]
	mov	pc, lr
	
	.globl asm_puts
asm_puts:
	stmfd	sp!, {r1-r3, lr}
	mov 	r3, r0
0:	ldrb 		r0, [r3]
	cmp 	r0,#0
	beq	2f
	cmp 	r0, # '\n'
	bne 1f
	mov 	r0, # '\r'
	bl asm_putc
	mov 	r0, # '\n'
1:
	bl asm_putc
	add  	r3, r3, #1
	b   0b
2:
	ldmfd	sp!, {r1-r3, lr}	
	mov 	pc, lr

	.globl asm_print_hex
asm_print_hex:
	stmfd	sp!, {r1-r4, lr}
	/* mov r5,lr */
	mov r3,r0

	mov r0,#'0'
	bl    asm_putc
	mov r0,#'x'
	bl    asm_putc

	mov r4,#8
1:	mov r3,r3, ror #28
	and r0, r3,#0x0F
	cmp r0,#9
	addls r0,r0,#'0'
	addhi r0,r0,#55
	bl    asm_putc
	subs r4, r4,#1
	bne 1b
	/* mov lr,r5 */
	ldmfd	sp!, {r1-r4, lr}	
	mov pc,lr
	
/*
 * NAND Interface init for SMDK6400
 */
nand_asm_init:
	ldr	r0, =ELFIN_NAND_BASE
	ldr	r1, [r0, #NFCONF_OFFSET]
	orr	r1, r1, #0x70
	orr	r1, r1, #0x7700
	str     r1, [r0, #NFCONF_OFFSET]

	ldr	r1, [r0, #NFCONT_OFFSET]
	orr	r1, r1, #0x03
	str     r1, [r0, #NFCONT_OFFSET]

	mov	pc, lr

	.global lcd_backlight_switch
lcd_backlight_switch:
	mov r2, r0
    ldr r0,=ELFIN_GPIO_BASE 
    ldr r1,[r0, #GPFCON_OFFSET]  @GPFCON in OK6410 board
    bic r1,r1,#(3<<28)
    orr r1,r1,#(1<<28)           @set GPF14 OUT mode
    str r1,[r0, #GPFCON_OFFSET]
    ldr r1,[r0, #GPFPUD_OFFSET]
    bic r1,r1,#(3<<28)            @disable pull up/down
    str r1,[r0, #GPFPUD_OFFSET]
    ldr r1,[r0, #GPFDAT_OFFSET]   @GPFDAT
    bic r1,r1,#(1<<14)
	cmp r2,#0x00
	orrne r1,r1,#(1<<14)
    str r1,[r0, #GPFDAT_OFFSET]   @set GPF14 output 
    mov pc,lr


#ifdef CONFIG_ENABLE_MMU
/*
 * MMU Table for SMDK6400
 */

	/* form a first-level section entry */
.macro FL_SECTION_ENTRY base,ap,d,c,b
	.word (\base << 20) | (\ap << 10) | \
	      (\d << 5) | (1<<4) | (\c << 3) | (\b << 2) | (1<<1)
.endm

.section .mmudata, "a"
	.align 14
	/* the following alignment creates the mmu table at address 0x4000. */
	.globl mmu_table
mmu_table:
	.set __base, 0
	/* 1:1 mapping for debugging */
	.rept 0xA00
	FL_SECTION_ENTRY __base, 3, 0, 0, 0
	.set __base, __base + 1
	.endr

	/* access is not allowed. */
	.rept 0xC00 - 0xA00
	.word 0x00000000
	.endr

	/* 256MB for SDRAM 0xC0000000 -> 0x50000000 */
	.set __base, 0x500
	.rept 0xD00 - 0xC00
	FL_SECTION_ENTRY __base, 3, 0, 1, 1
	.set __base, __base + 1
	.endr

	/* access is not allowed. */
	.rept 0x1000 - 0xD00
	.word 0x00000000
	.endr
#endif
