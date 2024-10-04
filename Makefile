CC=sdcc
# BUILDFLAGS:
# -DSTC89C52	required
# -DQX		for testing on QX51-mini development board
# -DX...	setting to a coded crystal frequency
# -DDS3231	use DS3231 RTC
# -DSIXSEGMENT	six segment display
# -DRAISEDZERO	upper loop used for 0 in six segment mode
#
# Example:
# BUILDFLAGS=-DSTC89C52 -DQX -DX11_059_200 -DDS3231
#
BUILDFLAGS=-DSTC89C52 -DX12_000_000 # -DSIXSEGMENT -DRAISEDZERO # -DDS3231
CFLAGS=-mmcs51 -Ipt-1.4 $(BUILDFLAGS) -DBUILDFLAGS="$(BUILDFLAGS)"
# needed for newer 89C52, see https://github.com/grigorig/stcgal/issues/50
#ARCH=stc12
INCLUDES=
LIBS=
PORT=/dev/ttyUSB0

clock.hex:	clock.rel ds3231.rel i2c.rel stc89c52.rel
		$(CC) -o $@ $^

clock.rel:	clock.c stc89c52.h ds3231.h
		$(CC) -c $(CFLAGS) $(INCLUDES) clock.c

ds3231.rel:	ds3231.c i2c.h
		$(CC) -c $(CFLAGS) $(INCLUDES) ds3231.c

%.asm:		%.c
		$(CC) $(CFLAGS) $(INCLUDES) -S $<

%.rel:		%.c %.h
		$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $(<:.c=.rel)

%.hex:		%.rel
		$(CC) $(CFLAGS) -o $@ $<

%.flash:	%.hex
ifndef ARCH
		stcgal -p $(PORT) $<
else
		stcgal -P $(ARCH) -p $(PORT) $<
endif

clean:
		rm -f *.{asm,sym,lst,rel,rst,sym,lk,map,mem}
