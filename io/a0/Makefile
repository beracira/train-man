#
# Makefile for busy-wait IO tests
#
#XCC     = gcc
#AS	= as
#LD      = ld

XCC = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc
AS	= /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as
LD  = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld

CFLAGS  = -c -fPIC -Wall -I. -I../include -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map a0.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

all:  a0.s a0.elf

a0io.s: a0io.c a0io.h
		$(XCC) -S $(CFLAGS) a0io.c

a0io.o: a0io.s
	$(AS) $(ASFLAGS) -o a0io.o a0io.s

a0.s: a0.c a0.h
	$(XCC) -S $(CFLAGS) a0.c

a0.o: a0.s
	$(AS) $(ASFLAGS) -o a0.o a0.s

a0.elf: a0.o a0io.o
	$(LD) $(LDFLAGS) -o $@ a0.o a0io.o -lbwio -lgcc

clean:
	-rm -f a0.elf *.s *.o a0.map
