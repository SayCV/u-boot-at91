/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_aic.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/io.h>
#include <div64.h>

unsigned long get_mck_clk_rate(void);

#ifdef CONFIG_USE_IRQ
#define CONFIG_TIMER_IRQ
#endif

#ifdef CONFIG_TIMER_IRQ
/*
 * When interrupts are enabled, use timer 2 for time/delay generation...
 */

static volatile ulong timestamp;

static void timer_isr(void)
{
	u32  status;
	at91_pit_t *pit = (at91_pit_t *) AT91_PIT_BASE;
                                                          		/* Clear the PIT interrupt                            */
	status = readl(&pit->pivr);
	(void)status;                                               /* Prevent a compiler warning that status was ...     */
                                                           		/* ... never used                                     */
                                                           		
	timestamp++;
}

ulong get_timer (ulong base)
{
	return timestamp - base;
}

void reset_timer (void)
{
	timestamp = 0;
}

int timer_init (void)
{
	u32  				cnts;
	u32  				mclk_freq;
	u32  				tick_per_sec	=	CONFIG_SYS_HZ;
	at91_pmc_t 	*pmc 					= (at91_pmc_t *) AT91_PMC_BASE;
	at91_pit_t 	*pit 					= (at91_pit_t *) AT91_PIT_BASE;
                                                                /* Determine the number of counts per tick.           */
	mclk_freq = get_mck_clk_rate();

	cnts      = ((mclk_freq) / 16 / tick_per_sec / 1000) - 1;			/* 1 ticks = 1us */
    
	/*
	 * Enable PITC Clock
	 * The clock is already enabled for system controller in boot
	 */
	writel(1 << AT91_ID_SYS, &pmc->pcer);
                                                                /* Set the vector address for PIT.                    */
	BSP_IntClr(AT91_ID_SYS);

	irq_install_handler((u8   )AT91_ID_SYS,
											(u8   )BSP_INT_PRIO_LOWEST,
											(u8   )BSP_INT_SCR_TYPE_INT_HIGH_LEVEL_SENSITIVE,
											(CPU_FNCT_VOID)timer_isr );

	BSP_IntEn(AT91_ID_SYS);

                                                                /* Enable the PIT with the correct compare value.     */
	writel(cnts | AT91_PIT_PITEN | AT91_PIT_PITIEN, &pit->mr);

	return 0;
}

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = usec * ((get_mck_clk_rate() >> 4)/1000000);
	tmp = timestamp + tmo;	/* get current timestamp */

	while (timestamp < tmp)	/* loop till event */
		 /*NOP*/;
}

#else
/*
 * We're using the AT91CAP9/SAM9 PITC in 32 bit mode, by
 * setting the 20 bit counter period to its maximum (0xfffff).
 */
#define TIMER_LOAD_VAL	0xfffff

static long long timestamp;
static ulong lastinc;
static ulong timer_freq;

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, timer_freq);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= timer_freq;
	do_div(usec, 1000000);

	return usec;
}

/* nothing really to do with interrupts, just starts up a counter. */
int timer_init(void)
{
	at91_pmc_t *pmc = (at91_pmc_t *) AT91_PMC_BASE;
	at91_pit_t *pit = (at91_pit_t *) AT91_PIT_BASE;
	/*
	 * Enable PITC Clock
	 * The clock is already enabled for system controller in boot
	 */
	writel(1 << AT91_ID_SYS, &pmc->pcer);

	/* Enable PITC */
	writel(TIMER_LOAD_VAL | AT91_PIT_MR_EN , &pit->mr);

	reset_timer_masked();

	timer_freq = get_mck_clk_rate() >> 4;

	return 0;
}

/*
 * timer without interrupts
 */
unsigned long long get_ticks(void)
{
	at91_pit_t *pit = (at91_pit_t *) AT91_PIT_BASE;

	ulong now = readl(&pit->piir);

	if (now >= lastinc)	/* normal mode (non roll) */
		/* move stamp forward with absolut diff ticks */
		timestamp += (now - lastinc);
	else			/* we have rollover of incrementer */
		timestamp += (0xFFFFFFFF - lastinc) + now;
	lastinc = now;
	return timestamp;
}

void reset_timer_masked(void)
{
	/* reset time */
	at91_pit_t *pit = (at91_pit_t *) AT91_PIT_BASE;

	/* capture current incrementer value time */
	lastinc = readl(&pit->piir);
	timestamp = 0; /* start "advancing" time stamp from 0 */
}

ulong get_timer_masked(void)
{
	return tick_to_time(get_ticks());
}

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = usec_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked () - base;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return timer_freq;
}
#endif
