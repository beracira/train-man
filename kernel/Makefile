#
# Makefile
#

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

LDFLAGS = -init main -Map kernel.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../io/lib

all:  kernel.s kernel.elf

kernel.s: kernel.c kernel.h
	$(XCC) -S $(CFLAGS) kernel.c

kernel.o: kernel.s
	$(AS) $(ASFLAGS) -o kernel.o kernel.s

syscall.s: syscall.c syscall.h
	$(XCC) -S $(CFLAGS) syscall.c

syscall.o: syscall.s
	$(AS) $(ASFLAGS) -o syscall.o syscall.s

functions.s: functions.c functions.h
	$(XCC) -S $(CFLAGS) functions.c

functions.o: functions.s
	$(AS) $(ASFLAGS) -o functions.o functions.s

priorityqueue.s: priorityqueue.c priorityqueue.h
	$(XCC) -S $(CFLAGS) priorityqueue.c

priorityqueue.o: priorityqueue.s
	$(AS) $(ASFLAGS) -o priorityqueue.o priorityqueue.s

kernel.elf: kernel.o functions.o syscall.o priorityqueue.o
	$(LD) $(LDFLAGS) -o $@ kernel.o functions.o syscall.o priorityqueue.o -lbwio -lgcc

clean:
	-rm -f kernel.elf *.s *.o kernel.map
