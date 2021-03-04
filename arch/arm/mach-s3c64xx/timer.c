/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/proc-armv/ptrace.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>

//#define  pr_debug  pr_err  //for debug

DECLARE_GLOBAL_DATA_PTR;

#define T4_PREDIV 2  /* PLK/4 */
#define T4_PRESCALER 165
#define T4_HZ 1 
#define T4_MUL 1000
int timer_init(void)
{
	S3C64XX_TIMERS *const timers = S3C64XX_GetBase_TIMERS();
	ulong pre_freq;
	
	/* use PWM Timer 4 because it has no output */
	/*
	 * We use the following scheme for the timer:
	 * Prescaler is hard fixed at 167, divider at 1/4.
	 * This gives at PCLK frequency 66MHz approx. 10us ticks
	 * The timer is set to wrap after 100s, at 66MHz this obviously
	 * happens after 10,000,000 ticks. A long variable can thus
	 * keep values up to 40,000s, i.e., 11 hours. This should be
	 * enough for most uses:-) Possible optimizations: select a
	 * binary-friendly frequency, e.g., 1ms / 128. Also calculate
	 * the prescaler automatically for other PCLK frequencies.
	 */
	timers->TCFG0 = T4_PRESCALER << 8;
	timers->TCFG1 = (timers->TCFG1 & ~0xf0000) | (T4_PREDIV << 16); 
	
	pre_freq = get_pwm_clk() / ((T4_PRESCALER+1) << T4_PREDIV);
	pr_debug("%s, timer final input freq: %ld Hz\n",__func__,pre_freq);
	gd->arch.timer_rate_hz = pre_freq/T4_HZ; /* tick per T4_HZ */
	pr_debug("%s, timer count per %d Hz : %ld\n", __func__,T4_HZ,gd->arch.timer_rate_hz);
	gd->arch.timer_rate_hz *= T4_MUL;  /* tick x T4_MUL */

	/* load value for 10 ms timeout */
	gd->arch.lastinc = timers->TCNTB4 = gd->arch.timer_rate_hz;
	/* auto load, manual update of Timer 4 */
	timers->TCON = (timers->TCON & ~0x00700000) | TCON_4_AUTO | TCON_4_UPDATE;

	/* auto load, start Timer 4 */
	timers->TCON = (timers->TCON & ~0x00700000) | TCON_4_AUTO | COUNT_4_ON;

    /* Use this as the current monotonic time in us */
	gd->arch.timer_reset_value = 0;

	return 0;
}

static inline ulong s3c_read_timer(void)
{
	S3C64XX_TIMERS *const timers = S3C64XX_GetBase_TIMERS();

	return timers->TCNTO4;
}

static unsigned long long s3c_get_timer(void)
{
	ulong now = s3c_read_timer();

	if (gd->arch.lastinc >= now) {
		/* normal mode */
		gd->arch.timer_reset_value += gd->arch.lastinc - now;
	} else {
		/* we have an overflow ... */
		gd->arch.timer_reset_value += gd->arch.lastinc + gd->arch.timer_rate_hz - now;
	}
	gd->arch.lastinc = now;

	return gd->arch.timer_reset_value;
}

unsigned long get_timer_masked(void)
{
	return s3c_get_timer();
}

void reset_timer_masked(void)
{
	/* reset time */
	gd->arch.lastinc = s3c_read_timer();
	gd->arch.timer_reset_value = 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = (usec + 9) / 10;     /* timer clk is 10 us per cycle  */
	tmp = s3c_get_timer() + tmo;  /* get current timer_reset_value */
	pr_debug("%s, wait: %ld us, end tmp: %lld\n",__func__,usec,tmp);

	while (s3c_get_timer() < tmp);/* loop till event */
	pr_debug("%s, cur tick: %lld\n", __func__,s3c_get_timer());
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return (ulong)(gd->arch.timer_rate_hz * T4_HZ / T4_MUL);  /* ticks per second */
}

void set_timer(ulong t) /* unit: 1 ms when SYS_HZ=1000 */
{
	gd->arch.timer_reset_value = t * get_tbclk()/ CONFIG_SYS_HZ;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return s3c_get_timer() * CONFIG_SYS_HZ / get_tbclk();
}

ulong get_timer(ulong base)  /* unit: 1 ms when SYS_HZ=1000 */
{
	return get_ticks() - base;
}
