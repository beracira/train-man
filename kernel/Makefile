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

LDFLAGS = -init main -Map kernel.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

all:  kernel.s kernel.elf

kernelio.s: kernelio.c kernelio.h
		$(XCC) -S $(CFLAGS) kernelio.c

kernelio.o: kernelio.s
	$(AS) $(ASFLAGS) -o kernelio.o kernelio.s

kernel.s: kernel.c kernel.h
	$(XCC) -S $(CFLAGS) kernel.c

kernel.o: kernel.s
	$(AS) $(ASFLAGS) -o kernel.o kernel.s

kernel.elf: kernel.o kernelio.o
	$(LD) $(LDFLAGS) -o $@ kernel.o kernelio.o -lbwio -lgcc

clean:
	-rm -f kernel.elf *.s *.o kernel.map
