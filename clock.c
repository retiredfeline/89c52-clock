//
// 89C52 based clock
// See README.md and LICENSE.md
//

#include <8051.h>

#include "stc89c52.h"
#include "pt.h"

#include "clock.h"

#define	DS3231		// using RTC
#ifdef	DS3231
#include "ds3231.h"
#endif

#ifdef	QX
#define	SEGMENTS	P0
#define	DIGITS		P2
#define	SWITCHES	P3
#else
#define	SEGMENTS	P2
#define	SEGMENTS_M0	P2M0
#define	DIGITS		P0
#define	SWITCHES	P3
#endif	// QX

#define	DISPLAYLEN	4			// digits in display (4/6)
#define	COLONBLINK				// blink colon or dp

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
#define	TICKSINHALFSEC	(TICKSINSEC/2)
#define	TICK		4			// ms, roughly
#define	DEPMIN		(100 / TICK)		// debounce period
#define	RPTTHRESH	((400 / TICK) + 1)	// repeat threshold after debounce
#define	RPTPERIOD	(250 / TICK)		// repeat period

// SWITCHES
#define	MINBUTTON	0x04			// .2
#define	HOURBUTTON	0x08			// .3
#define	MODE		0x10			// .4
#define	BRIGHTNESS	0x20			// .5
#define	SWMASK		(MINBUTTON|HOURBUTTON|MODE|BRIGHTNESS)

uchar tickdiv, ticks, colon;
uchar now[7];					// matches DS3231 layout
#define	SEC		now[0]
#define	SECIDX		0
#define	MIN		now[1]
#define	MINIDX		1
#define	HOUR		now[2]
#define	HOURIDX		2
#define	DATE		now[4]
#define	DATEIDX		4
#define	MONTH		now[5]
#define	MONTHIDX	5
#define	YEAR		now[6]
#define	YEARIDX		6
uchar segments[6];				// 7 segment data of HHMMSS
uchar currdig;					// digit to load
uchar brightlevel, brightness;			// index, value

uchar swstate, swtent, swmin, swrepeat;		// switch handling
enum Mode { Time, Date } mode;
struct pt pt;

__code uchar font[] = {				// LSB = a, MSB = dp
#ifdef	QX
	// 0 bit means that segment is on
	0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,
	0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf,
#else
	// 1 bit means that segment is on
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,
	0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
#endif
};

#define	DIGITS_OFF	0xff
__code uchar digmask[] = {
	// LSB = MSD, 0 bit means that digit is on
	0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF,
};

// Lookup table
__code uchar byte2bcd[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
};

// Brightness levels
__code uchar brightlevels[] = {
	// 0 is maximum brightness
	(TICKDIV*7)/8, (TICKDIV*3)/4, TICKDIV/2, 0,
};
#define	NLEVELS	(sizeof brightlevels/sizeof brightlevels[0])

__code uchar builddate[] = __DATE__;

void timer0(void) __interrupt(1)
{
	tickdiv--;	// TL0 will reload from TH0
}

static uchar incmin(void)
{
	MIN++;
	if (MIN >= 60) {
		MIN = 0;
		return 1;
	}
	return 0;
}

static void inchour(void)
{
	HOUR++;
	if (HOUR >= 24)
		HOUR = 0;
}

static void incdate(void)
{
	DATE++;
	if (DATE > 31)
		DATE = 1;
}

static void incmonth(void)
{
	MONTH++;
	if (MONTH > 12)
		MONTH = 1;
}

static void bcd2segment(uchar value, uchar *segments)
{
	*segments++ = font[(value >> 4) & 0xF];
	*segments = font[value & 0xF];
}

static void updatedisplay(void)
{
#ifdef	TESTING
	if (HOUR > 24)
		HOUR = 24;
	if (MIN > 60)
		MIN = 60;
	if (SEC > 60)
		SEC = 60;
#endif
	switch (mode) {
	case Time:
		bcd2segment(byte2bcd[HOUR], &segments[0]);
#ifdef	COLONBLINK
#ifdef	QX
		segments[1] &= ~colon;
#else
		segments[1] |= colon;
#endif	// QX
#endif	// COLONBLINK
		bcd2segment(byte2bcd[MIN], &segments[2]);
		bcd2segment(byte2bcd[SEC], &segments[4]);
		break;
	case Date:
		bcd2segment(byte2bcd[DATE], &segments[0]);
		bcd2segment(byte2bcd[MONTH], &segments[2]);
		bcd2segment(byte2bcd[YEAR], &segments[4]);
		break;
	}
}

static void switchaction()
{
	switch(~swstate & SWMASK) {
	case BRIGHTNESS:
		brightlevel++;
		if (brightlevel >= NLEVELS)
			brightlevel = 0;
		brightness = brightlevels[brightlevel];
		break;
	case MODE:
		switch (mode) {
		case Time:
#ifdef	DS3231
			mode = Date;		// only for RTC
#endif
			break;
		case Date:
		default:
			mode = Time;
			break;
		}
		break;
	case MINBUTTON:
		switch (mode) {
		case Time:
			SEC = 0;
#ifdef	DS3231
			writereg(0, SECIDX);
#endif
			(void)incmin();
#ifdef	DS3231
			writereg(byte2bcd[MIN], MINIDX);
#endif
			break;
		case Date:
			incdate();
#ifdef	DS3231
			writereg(byte2bcd[DATE], DATEIDX);
#endif
			break;
		}
		updatedisplay();
		break;
	case HOURBUTTON:
		switch (mode) {
		case Time:
#ifdef	DS3231
			writereg(byte2bcd[SEC], SECIDX);
#endif
			inchour();
#ifdef	DS3231
			writereg(byte2bcd[HOUR], HOURIDX);
#endif
			break;
		case Date:
			incmonth();
#ifdef	DS3231
			writereg(byte2bcd[MONTH], MONTHIDX);
#endif
			break;
		}
		updatedisplay();
		break;
	}
}

static inline void reinitstate()
{
	swtent = swstate;
	swmin = DEPMIN;
	swrepeat = RPTTHRESH;
}

static
PT_THREAD(switchhandler(struct pt *pt))
{
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, swstate != swtent);
	swtent = swstate;
	PT_WAIT_UNTIL(pt, --swmin <= 0 || swstate != swtent);
	if (swstate != swtent) {		// changed, restart
		reinitstate();
		PT_RESTART(pt);
	}
	switchaction();
	PT_WAIT_UNTIL(pt, --swrepeat <= 0 || swstate != swtent);
	if (swstate != swtent) {		// changed, restart
		reinitstate();
		PT_RESTART(pt);
	}
	switchaction();
	for (;;) {
		swrepeat = RPTPERIOD;
		PT_WAIT_UNTIL(pt, --swrepeat <= 0 || swstate == SWMASK);
		if (swstate == SWMASK) {	// released, restart
			reinitstate();
			PT_RESTART(pt);
		}
		switchaction();
	}
	PT_END(pt);
}

static void scandisplay(void)
{
	SEGMENTS = segments[currdig];
	DIGITS = digmask[currdig];
	currdig++;
	if (currdig > DISPLAYLEN)
		currdig = 0;
}

void main(void)
{
#ifndef	QX
	// set SEGMENTS port to push-pull
	SEGMENTS_M0 = 0xFF;
#endif
	TMOD = T0_M1;			// mode 2 on T0
	TH0 = COUNT_TL0;		// load recurring divisor
	TL0 = COUNT_TL0;		// overflow next cycle
	ET0 = 1;			// enable T0 interrupts
	TR0 = 1;			// turn on T0
	EA = 1;				// enable global interrupts
	swstate = SWMASK;		// all buttons up
	reinitstate();			// switch handler
	mode = Time;
	tickdiv = TICKDIV;
	ticks = 0;
	colon = 0x0;
#ifdef	DS3231
	SEC = MIN = HOUR = 0;
	ds3231_init();
#else
	SEC = 56;
	MIN = 34;
	HOUR = 12;
	DATE = 1;
	MONTH = 1
	YEAR = 20;
#endif
	brightlevel = 0;
	brightness = brightlevels[0];
	updatedisplay();		// load segments
	currdig = 0;			// start scan at LHD
	for (;;) {
		while (tickdiv > 0) {	// PWM
			if (tickdiv < brightness)
				DIGITS = DIGITS_OFF;	// turn off
		}
		tickdiv = TICKDIV;
		ticks++;
		if (ticks >= TICKSINHALFSEC && colon == 0x0) {
			colon = 0x80;
			updatedisplay();
		}
		if (ticks >= TICKSINSEC) {
			ticks = 0;
			colon = 0x0;
#ifdef	DS3231
			DIGITS = DIGITS_OFF;	// blank display temporarily
			getnow(now);
			DIGITS = digmask[currdig];
#else
			SEC++;
			if (SEC >= 60) {
				SEC = 0;
				if (incmin())
					inchour();
			}
#endif
			updatedisplay();
		}
		scandisplay();
		swstate = SWITCHES & SWMASK;
		PT_SCHEDULE(switchhandler(&pt));
	}
}
