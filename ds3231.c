#include "clock.h"
#include "i2c.h"
#include "ds3231.h"

void ds3231_init(void)
{
	i2cinit();
}

inline static uchar bcd2bin(uchar c)
{
	return c - 6 * (c >> 4);
}

void getnow(uchar *now)
{
	uchar *p = now;
	i2cstart();
	i2csendaddr();
	i2csend(0);
	i2crestart();
	i2creadaddr();
	for (uchar i = 0; i < 6; i++) {
		*p++ = bcd2bin(i2cread());
		i2cack();
	}
	*p = bcd2bin(i2cread());
	i2cnak();
	i2cstop();
	if (now[2] >= 24)
		now[2] = 24;
	if (now[1] >= 60)
		now[1] = 60;
	if (now[0] >= 60)
		now[0] = 60;
}

void writereg(uchar value, uchar reg)
{
	i2cstart();
	i2csendaddr();
	i2csend(reg);
	i2csend(value);
	i2cstop();
}
