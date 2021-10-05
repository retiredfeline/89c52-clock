CC=sdcc
BUILDFLAGS=-DQX -DX11_059_200 -DDS3231
# BUILDFLAGS=-DX12_000_000 -DSIXSEGMENT # -DRAISEDZERO
CFLAGS=-mmcs51 -Ipt-1.4 $(BUILDFLAGS) -DBUILDFLAGS="$(BUILDFLAGS)"
# needed for newer 89C52, see https://github.com/grigorig/stcgal/issues/50
#ARCH=stc12
INCLUDES=
LIBS=
PORT=/dev/ttyUSB0

clock.ihx:	clock.rel ds3231.rel i2c.rel
		$(CC) -o $@ $^

clock.rel:	clock.c stc89c52.h ds3231.h
		$(CC) -c $(CFLAGS) $(INCLUDES) clock.c

ds3231.rel:	ds3231.c i2c.h
		$(CC) -c $(CFLAGS) $(INCLUDES) ds3231.c

%.asm:		%.c
		$(CC) $(CFLAGS) $(INCLUDES) -S $<

%.rel:		%.c %.h
		$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $(<:.c=.rel)

%.ihx:		%.rel
		$(CC) $(CFLAGS) -o $@ $<

%.flash:	%.ihx
ifndef ARCH
		stcgal -p $(PORT) $<
else
		stcgal -P $(ARCH) -p $(PORT) $<
endif

clean:
		rm -f *.{asm,sym,lst,rel,rst,sym,lk,map,mem,ihx}
