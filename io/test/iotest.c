 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>

int handle(void) {
	return 0;
}

// allocate all memory for user tasks. i.e. task_queue
// create a first user task, puts it into the queue
int init() {

}

int foo(void) {	bwprintf( COM2, "Hello world.\n\r" ); }

struct TD{
	int sp;
	int pc;
	int lr;
	int reg[32];
	TD * next;
	int tid;
	int parent;
	int priority;
	int state;
	int exit_code;
};

// 2 mb per task (roughly)
TD * task_queue[16];
TD * schedule() {
	int i = 0;
	while (!task_queue[i]->ready) ++i;
	return task_queue[i];
}

// context switch (loading registers)
int activate (TD * td) {
	// tbd

}

int main( int argc, char* argv[] ) {
	int * entry = (int *) 0x28;
	*entry = 0x00218000;

	unsigned int val, pc;
	asm (
		"mov %0, LR;"
		: "=r" ( val )        /* output */
	 );
	bwprintf(COM2, "LR: %u\n\r", val);

	// int * entry2 = (int *) 0x2c;
	// *entry2 = (int) &handle;
	asm (
		"MRS R0, CPSR;"
		"mov %0, R0;"
		: "=r" ( val )        /* output */
	 );
	asm (
		"mov %0, PC;"
		: "=r" ( pc )
	);
	bwprintf(COM2, "handle: %u\n\r", &handle);
	bwprintf(COM2, "%u %u\n\r", val, pc);
	asm (
		"mov R0, #0xd0;"
		"MSR CPSR, R0;"
		"MRS R1, CPSR;"
		"mov %0, R1;"
		: "=r" ( val )        /* output */
	 );
	asm (
		"mov %0, PC;"
		: "=r" ( pc )
	);
	bwprintf(COM2, "%u %u\n\r", val, pc);
	asm (
		"swi 0;"
	);
	bwprintf(COM2, "returned from handle\n\r");
	return 0;
}

