//
// I2C communication functions using bit-banging
// I have implemented only what I needed for my project
// In particular, clock stretching is not implemented
// Feel free to improve
//

#include <8051.h>

#include "stc89c52.h"

#include "i2c.h"

#define	EXT_PULLUP

// call and ret take 2 us each, add a nop for 5 us total
// add more nops if crystal > 12 MHz
static void delay5us() __naked
{
	__asm__("nop");
	__asm__("ret");
}

void i2cinit() __naked
{
#ifdef	EXT_PULLUP
	P1M0 = 0x0A;		// P1M0.2,3 = 1, open drain
	P1M1 = 0x0A;		// P1M1.2,3 = 1, open drain
#endif
	SDA = 1;
	delay5us();
	SCL = 1;
	delay5us();
	__asm__("ret");
}

void i2cstart() __naked
{
	SDA = 0;
	delay5us();
	SCL = 0;
	delay5us();
	__asm__("ret");
}

void i2crestart() __naked
{
	SDA = 1;
	delay5us();
	SCL = 1;
	delay5us();
	SDA = 0;
	delay5us();
	SCL = 0;
	delay5us();
	__asm__("ret");
}

void i2cstop() __naked
{
	SCL = 0;
	delay5us();
	SDA = 0;
	delay5us();
	SCL = 1;
	delay5us();
	SDA = 1;
	delay5us();
	__asm__("ret");
}

void i2cack() __naked
{
	SDA = 0;
	delay5us();
	SCL = 1;
	delay5us();
	SCL = 0;
	delay5us();
	SDA = 1;
	delay5us();
	__asm__("ret");
}

void i2cnak() __naked
{
	SDA = 1;
	delay5us();
	SCL = 1;
	delay5us();
	SCL = 0;
	delay5us();
	SDA = 1;
	delay5us();
	__asm__("ret");
}

unsigned char i2csendaddr()
{
	return i2csend(ADDR << 1);
}

unsigned char i2creadaddr()
{
	return i2csend((ADDR << 1) | 1);
}

unsigned char i2csend(unsigned char data)
{
	unsigned char i;

	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			SDA = 1;
		else
			SDA = 0;
		delay5us();
		SCL = 1;
		delay5us();
		SCL = 0;
		data <<= 1;
	}
	SDA = 1;
	delay5us();
	SCL = 1;
	i = SDA;
	delay5us();
	SCL = 0;
	return i;
}

unsigned char i2cread()
{
	unsigned char i;
	unsigned char data = 0;

	for (i = 0; i < 8; i++) {
		data <<= 1;
		data |= SDA;
		SCL = 1;
		delay5us();
		SCL = 0;
	}
	return data;
}
