#
# Makefile
#

XCC = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc
AS	= /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as
LD  = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld

CFLAGS  = -c -fPIC -Wall -O0 -Wextra -fno-builtin -I. -I../include -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map kernel.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../io/lib

EXEC = kernel.elf
OBJECTS = kernel.o common.o functions.o syscall.o priorityqueue.o td.o user_syscall.o nameserver.o irq.o

%.o: %.c
	$(XCC) -S $(CFLAGS) $<
	$(AS) $(ASFLAGS) -o $@ ${@:.o=.s}

${EXEC}: ${OBJECTS}
	${LD} ${LDFLAGS} -o $@ ${OBJECTS} -lbwio -lgcc

clean:
	-rm -f kernel.elf *.s *.o kernel.map *.d
