#ifndef STC89C52_H
#define STC89C52_H

#define	_SDCC_

// Crystal frequency

#ifdef	X11_059_200
#define	COUNTDIV	240			// 11.0592 MHz -> 3840 Hz
#define	TICKDIV		16			// 3840 Hz -> 240 Hz
#define	TICKSINSEC	240
#endif

#ifdef	X12_000_000
#define	COUNTDIV	250			// 12 MHz -> 4000 Hz
#define	TICKDIV		16			// 4000 Hz -> 250 Hz
#define	TICKSINSEC	250
#endif

#define COUNT_TL0	(256 - COUNTDIV)

typedef unsigned char tick_t;

#include "stc89.h"

/* Port mode bits */

/*  BYTE Register  */
__sfr __at (0x91) P1M1 ;
__sfr __at (0x92) P1M0 ;
__sfr __at (0x93) P0M1 ;
__sfr __at (0x94) P0M0 ;
__sfr __at (0x95) P2M1 ;
__sfr __at (0x96) P2M0 ;
__sfr __at (0xB1) P3M1 ;
__sfr __at (0xB2) P3M0 ;

/* External interrupt timer 2 */
__sbit __at (0xAD) ET2 ;

/* T2CON bits */
__sbit __at (0xCA) TR2 ;
__sbit __at (0xCB) EXEN2 ;
__sbit __at (0xCF) TF2 ;

#define	TF2_VECTOR	5

#ifdef	QX
#define	SEGMENTS	P0
#define	DIGITS		P2
#define	SWITCHES	P3
#define	LOWON
#else
#define	SEGMENTS	P2
#define	SEGMENTS_M0	P2M0
#define	DIGITS		P0
#define	SWITCHES	P3
#endif	// QX

#define	I2C_ADDR	0x68U
#define	SCL		P1_2
#define	SDA		P1_3
#define	SCL_L()		P1_2 = 0
#define	SCL_H()		P1_2 = 1
#define	SDA_L()		P1_3 = 0
#define	SDA_H()		P1_3 = 1
#define	READ_SDA()	SDA

#define	DHT22D		P1_4
#define	DHT22L		P1_4 = 0
#define	DHT22H		P1_4 = 1
#define	READ_DHT22D	DHT22D

extern uchar volatile tickdiv;

extern void timer0(void) __interrupt(TF0_VECTOR);
extern void ports_init(void);
extern void timer_init(void);
extern void pullup_init(void);

#undef	TIMER_DELAY

extern void delay5us(void) __naked;

#define	ASMRET()	__asm__("ret")

#endif
