#include "clock.h"
#ifdef	DHT22
#include "dht22.h"
#endif

void timer0(void) __interrupt(TF0_VECTOR)
{
	tickdiv--;		// TL0 will reload from TH0
}

void ports_init(void)
{
#ifndef	LOWON
	// set SEGMENTS port to push-pull
	SEGMENTS_M0 = 0xFF;
#endif
}

void timer_init(void)
{
	TMOD = T0_M1 | T1_M0;	// mode 2 on T0, mode 1 on T1
	TH0 = COUNT_TL0;	// load recurring divisor
	TL0 = COUNT_TL0;	// overflow next cycle
	ET0 = 1;		// enable T0 interrupts
	TR0 = 1;		// turn on T0
	EA = 1;			// enable global interrupts
}

void pullup_init(void)
{
#ifdef	EXT_PULLUP
	P1M0 = 0x0A;		// P1M0.2,3 = 1, open drain
	P1M1 = 0x0A;		// P1M1.2,3 = 1, open drain
#endif
}

#ifdef	TIMER_DELAY

#define	SCANDIV		(65536-5)	// 5 us at least

void delay5us() __naked
{
	TH1 = SCANDIV >> 8;
	TL1 = SCANDIV & 0xff;
	TR1 = 1;		// start T1
	while (TF1 == 0)
		;		// wait for overflow
	TR1 = 0;		// turn off T1
	TF1 = 0;		// clear overflow
	__asm__("ret");
}

#endif	// TIMER_DELAY
