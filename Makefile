CC=sdcc
# BUILDFLAGS:
# -DSTC89C52	required
# -DQX		for testing on QX51-mini development board
# -DX...	setting to a coded crystal frequency
# -DDS3231	use DS3231 RTC
# -DDHT22	use DHT22 temperate and humidity sensor
# -DONLYDHT22	only DHT22 used, no clock
# -DSIXSEGMENT	six segment display
# -DRAISEDZERO	upper loop used for 0 in six segment mode
# -DDISPLAYLEN=4	can shorten to 2 if only 2 digits for DHT display
# -DTESTDATA	use synthetic one-wire test data to test decoding and display
#
# Example:
# BUILDFLAGS=-DSTC89C52 -DQX -DX11_059_200 -DDS3231
#
ifeq "$(QX)" "Y"
BUILDFLAGS=-DQX -DSTC89C52 -DX11_059_200 -DDS3231 -DDISPLAYLEN=4 # -DSIXSEGMENT -DRAISEDZERO # -DDS3231
.DEFAULT_GOAL=clock-qx.hex
else
BUILDFLAGS=-DSTC89C52 -DX12_000_000 -DDS3231 -DDISPLAYLEN=4 # -DSIXSEGMENT -DRAISEDZERO # -DDS3231
endif
CFLAGS=-mmcs51 -Ipt-1.4 $(BUILDFLAGS) -DBUILDFLAGS="$(BUILDFLAGS)"
# needed for newer 89C52, see https://github.com/grigorig/stcgal/issues/50
#ARCH=stc12
INCLUDES=
LIBS=
PORT=/dev/ttyUSB0

clock.hex:	clock.rel ds3231.rel i2c.rel dht22.rel stc89c52.rel
		$(CC) -o $@ $^

clock-qx.hex:	clock.rel ds3231.rel i2c.rel dht22.rel stc89c52.rel
		$(CC) -o $@ $^

clock.rel:	clock.c stc89c52.h ds3231.h dht22.h
		$(CC) -c $(CFLAGS) $(INCLUDES) clock.c

ds3231.rel:	ds3231.c i2c.h
		$(CC) -c $(CFLAGS) $(INCLUDES) ds3231.c

dht22.rel:	dht22.c dht22.h
		$(CC) -c $(CFLAGS) $(INCLUDES) dht22.c

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

.PHONY:		clean
clean:
		rm -f *.{asm,sym,lst,rel,rst,sym,lk,map,mem}
