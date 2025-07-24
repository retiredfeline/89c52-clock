#include "clock.h"
#include "stc89c52.h"
#include "dht22.h"

#define	TWOMSCOUNT	(65536U-1000U)

// These are not µs but counts determined by trial and error
#define	LO		2		// normally 27µs
#define	HI		8		// normally 70µs
#define	HILOTHRESH	5

#ifdef	TESTDATA
__code uchar hightime[40] = {		// 765 for humidity and 234 for temperature
// 765 = 0b 0000 0010 1111 1101
LO, LO, LO, LO, LO, LO, HI, LO,
HI, HI, HI, HI, HI, HI, LO, HI,
// 234 = 0b 0000 0000 1110 1010
LO, LO, LO, LO, LO, LO, LO, LO,
HI, HI, HI, LO, HI, LO, HI, LO,
// checksum = (2 + 253 + 0 + 234) & 0xFF = 233 = 0b11101001
HI, HI, HI, LO, HI, LO, LO, HI,
};
#else
__pdata uchar hightime[40];
#endif

void dht22_init(void)
{
	DHT22H;		// set line high to start
//	DCEN == 0;	// default counting up
	T2CON = 0;	// count up mode
	EXEN2 = 0;	// default no external signal
	RCAP2L = 0;	// reload value
	RCAP2H = 0;
}

// Convert 0-999 value into 2 BCD bytes, last nybble set to 0
void int2bcd(int value, char *result)
{
	if (value < 0)
		value = 0;	// sorry cannot display negative values
	if (value > 999)
		value = 999;	// clip at 999
	result[0] = 0;
	while (value >= 100) {
		result[0] += 0x10;
		value -= 100;
	}
	while (value >= 10) {
		result[0]++;
		value -= 10;
	}
	result[1] = value << 4;
}

static int collect40bits() {

	TL2 = TWOMSCOUNT & 0xFF;// 2 ms
	TH2 = TWOMSCOUNT >> 8;
	DHT22L;			// pull down
	TR2 = 1;		// turn on T2
	while (!TF2)
		;
	TR2 = 0;		// turn off T2
	TF2 = 0;		// clear overflow
	DHT22H;			// pull up
#ifndef	TESTDATA		// simulate delay anyway
	TL2 = 0;		// full 16 bits = ~65 ms timeout
	TH2 = 0;
	TF2 = 0;
	TR2 = 1;		// turn on T2
	while (!TF2 && DHT22D)	// wait for 0
		;
	while (!TF2 && !DHT22D)	// wait for 1
		;
	while (!TF2 && DHT22D)	// wait for 0
		;
	if (TF2) {		// timed out, faulty sensor?
		TR2 = 0;	// turn off T2
		TF2 = 0;	// clear overflow
		return 40;
	}
	for (uchar i = 0; i < 40; i++) {
		while (!TF2 && !DHT22D)	// wait for 1
			;
		uchar count = 0;
		while (!TF2 && DHT22D)	// wait for 0
			count++;
		hightime[i] = count;
		if (TF2) {	// timed out, didn't collect bits
			TR2 = 0;// turn off T2
			TF2 = 0;// clear overflow
			DHT22H;	// pull up
			return i + 1;
		}
	}
	TR2 = 0;		// turn off T2
	TF2 = 0;		// clear overflow
	DHT22H;			// pull up
#endif
	return 0;		// success
}

static void convert40bits(dhtresult *data)
{
	uchar	values[5], vi;

	for (uchar i = 0; i < 40; i++) {
		vi = i >> 3;
		if ((i & 0x07) == 0)
			values[vi] = 0;
		values[vi] <<= 1;
		values[vi] |= (hightime[i] > HILOTHRESH) ? 1 : 0;
	}
	if (((values[0] + values[1] + values[2] + values[3]) & 0xFF) != values[4])
		data->invalid = 2;	// checksum error
	else {
		data->humidity = ((int)values[0] << 8) + values[1];
		data->temperature = ((int)values[2] << 8) + values[3];
	}
}

void getdht22data(dhtresult *data)
{
	data->invalid = 0;
	if (collect40bits())
		data->invalid = 1;	// collection error
	else
		convert40bits(data);
}
