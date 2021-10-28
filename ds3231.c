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
	i2cstart();
        i2csendaddr();
        i2csend(0);
        i2cstop();

	i2cstart();
	i2creadaddr();
	for (int i = 0; i < 6; i++) {
		*now++ = bcd2bin(i2cread());
		i2cack();
	}
	*now++ = bcd2bin(i2cread());
	i2cnak();
        i2cstop();
}

void writereg(uchar value, uchar reg)
{
	i2cstart();
        i2csendaddr();
        i2csend(reg);
	i2csend(value);
	i2cstop();
}
