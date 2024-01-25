//
// I2C communication functions using bit-banging
// I have implemented only what I needed for my project
// In particular, clock stretching is not implemented
// Feel free to improve
//

#include "clock.h"
#include "i2c.h"

void i2cinit() __naked
{
	pullup_init();
	SDA_H();
	delay5us();
	SCL_H();
	delay5us();
	ASMRET();
}

void i2cstart() __naked
{
	SDA_L();
	delay5us();
	SCL_L();
	delay5us();
	ASMRET();
}

void i2crestart() __naked
{
	SDA_H();
	delay5us();
	SCL_H();
	delay5us();
	SDA_L();
	delay5us();
	SCL_L();
	delay5us();
	ASMRET();
}

void i2cstop() __naked
{
	SCL_L();
	delay5us();
	SDA_L();
	delay5us();
	SCL_H();
	delay5us();
	SDA_H();
	delay5us();
	ASMRET();
}

void i2cack() __naked
{
	SDA_L();
	delay5us();
	SCL_H();
	delay5us();
	SCL_L();
	delay5us();
	SDA_H();
	delay5us();
	ASMRET();
}

void i2cnak() __naked
{
	SDA_H();
	delay5us();
	SCL_H();
	delay5us();
	SCL_L();
	delay5us();
	SDA_H();
	delay5us();
	ASMRET();
}

unsigned char i2csendaddr()
{
	return i2csend(I2C_ADDR << 1);
}

unsigned char i2creadaddr()
{
	return i2csend((I2C_ADDR << 1) | 1);
}

unsigned char i2csend(unsigned char data)
{
	unsigned char i;

	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			SDA_H();
		else
			SDA_L();
		delay5us();
		SCL_H();
		delay5us();
		SCL_L();
		delay5us();
		data <<= 1;
	}
	SDA_H();
	delay5us();
	SCL_H();
	i = READ_SDA();
	delay5us();
	SCL_L();
	delay5us();
	return i;
}

unsigned char i2cread()
{
	unsigned char i;
	unsigned char data = 0;

	for (i = 0; i < 8; i++) {
		data <<= 1;
		data |= READ_SDA();
		SCL_H();
		delay5us();
		SCL_L();
		delay5us();
	}
	return data;
}
