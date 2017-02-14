#
# Makefile
#

XCC = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc
AS	= /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as
LD  = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld

CFLAGSO2  = -c -fPIC -Wall -O2 -Wextra -fno-builtin -I. -I../include -mcpu=arm920t -msoft-float
CFLAGSO0 = -c -fPIC -Wall -O0 -Wextra -fno-builtin -I. -I../include -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map kernel.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../io/lib

EXEC = kernel.elf
OBJECTS = kernel.o user_syscall.o common.o functions.o syscall.o priorityqueue.o td.o nameserver.o irq.o clockserver.o io.o train_ui.o

kernel.o: kernel.c
	$(XCC) -S $(CFLAGSO0) $<
	$(AS) $(ASFLAGS) -o $@ ${@:.o=.s}

user_syscall.o: user_syscall.c
	$(XCC) -S $(CFLAGSO0) $<
	$(AS) $(ASFLAGS) -o $@ ${@:.o=.s}

%.o: %.c
	$(XCC) -S $(CFLAGSO2) $<
	$(AS) $(ASFLAGS) -o $@ ${@:.o=.s}

${EXEC}: ${OBJECTS}
	${LD} ${LDFLAGS} -o $@ ${OBJECTS} -lbwio -lgcc

O0:
	make kernel.elf CFLAGSO2="-c -fPIC -Wall -O0 -Wextra -fno-builtin -I. -I../include -mcpu=arm920t -msoft-float"

clean:
	-rm -f kernel.elf *.s *.o kernel.map *.d
