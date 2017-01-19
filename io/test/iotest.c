 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>

// struct TD{
// 	int sp;
// 	int pc;
// 	int lr;
// 	//int reg[32];
// 	//TD * next;
// 	int tid;
// //	int parent;
// 	int priority;
// 	int state;
// 	int exit_code;
// };

// // 2 mb per task (roughly)
// TD td_list[1];
// int kernel_reg[32];


// int handle(void) {
// 	return 0;
// }

// // allocate all memory for user tasks. i.e. task_queue
// // create a first user task, puts it into the queue
// int init(void) {

// 	unsigned int sp;
// 	asm (
// 		"mov %0, SP;"
// 		: "=r" ( sp )
// 	);

// 	sp -= 4 * 1024 * 1024;

// 	td_list[0].sp = sp;
// 	td_list[0].pc = 0;
// 	td_list[0].lr = 0;
// 	td_list[0].tid = 1;
// 	td_list[0].priority = 2;
// 	td_list[0].state = 3;
// 	td_list[0].exit_code = 4;

// 	return 0;
// }

// int foo(void) {	
// 	bwprintf( COM2, "Hello world.\n\r" ); 
// 	return 0;
// }



// TD * schedule(void) {
// 	int i = 0;
// 	while (!task_queue[i]->ready) ++i;
// 	return task_queue[i];
// }

// //context switch (loading registers)
// int activate (TD * td) {
// 	// tbd
// 	return 0;
// }

int main( int argc, char* argv[] ) {
	int * entry = (int *) 0x28;
	*entry = 0x00218000;

	// int (*fooptr)(void);
	// fooptr = foo;
	// bwprintf(COM2, "foo: %u\n\r", foo);
	// bwprintf(COM2, "fooptr: %u\n\r", fooptr);
	// bwprintf(COM2, "&foo: %u\n\r", &foo);
	
	// foo();

	unsigned int sp, pc, lr, cpsr, spsr, fp, val;
	asm (
		"mov %0, PC;"
		: "=r" ( pc )
	);
	bwprintf(COM2, "PC: %u\n\r", pc);

	asm (
		"mov %0, SP;"
		: "=r" ( sp )
	);
	bwprintf(COM2, "SP: %u\n\r", sp);

	bwprintf(COM2, "LR input: 0x12\n\r");

	asm (
		"mov lr, #0x12;"
		"mov %0, lr;"
		: "=r" ( lr )
	);
	bwprintf(COM2, "LR: %u\n\r", lr);

	bwprintf(COM2, "LR input: 0x13\n\r", lr);

	asm (
		"mov lr, #0x13;"
		"mov %0, lr;"
		: "=r" ( lr )
	);
	bwprintf(COM2, "LR: %u\n\r", lr);

	asm (
		"mrs %0, CPSR;"
		: "=r" (cpsr )
	);
	bwprintf(COM2, "cpsr: %u\n\r", cpsr);

	asm (
		"mrs %0, SPSR;"
		: "=r" (spsr )
	);
	bwprintf(COM2, "spsr: %u\n\r", spsr);

	asm (
		"mov %0, FP;"
		: "=r" (fp )
	);
	bwprintf(COM2, "fp: %u\n\r", fp);

	asm (
		"mov %0, PC;"
		: "=r" ( pc )
	);
	bwprintf(COM2, "PC: %u\n\r", pc);

	asm (
		"mov R0, #0xd0;"
		"MSR CPSR, R0;"
		"MRS R1, CPSR;"
		"mov %0, R1;"
		: "=r" ( val )        /* output */
	 );

	bwprintf(COM2, "usr val: %u\n\r", val);

	asm (
		"mov %0, PC;"
		: "=r" ( pc )
	);
	bwprintf(COM2, "PC: %u\n\r", pc);

	asm (
		"mov %0, SP;"
		: "=r" ( sp )
	);
	bwprintf(COM2, "SP: %u\n\r", sp);

	asm (
		"mov %0, LR;"
		: "=r" ( lr )
	);
	bwprintf(COM2, "LR: %u\n\r", lr);

	asm (
		"mrs %0, CPSR;"
		: "=r" (cpsr )
	);
	bwprintf(COM2, "cpsr: %u\n\r", cpsr);

	asm (
		"mrs %0, SPSR;"
		: "=r" (spsr )
	);
	bwprintf(COM2, "spsr: %u\n\r", spsr);

	asm (
		"mov %0, FP;"
		: "=r" (fp )
	);
	bwprintf(COM2, "fp: %u\n\r", fp);

	asm (
		"mov %0, PC;"
		: "=r" ( pc )
	);
	bwprintf(COM2, "PC: %u\n\r", pc);
	

	// unsigned int val, pc;
	// asm (
	// 	"mov %0, LR;"
	// 	: "=r" ( val )        /* output */
	//  );
	// bwprintf(COM2, "LR: %u\n\r", val);

	// // int * entry2 = (int *) 0x2c;
	// // *entry2 = (int) &handle;
	// asm (
	// 	"MRS R0, CPSR;"
	// 	"mov %0, R0;"
	// 	: "=r" ( val )        /* output */
	//  );
	// asm (
	// 	"mov %0, PC;"
	// 	: "=r" ( pc )
	// );
	// bwprintf(COM2, "handle: %u\n\r", &handle);
	// bwprintf(COM2, "%u %u\n\r", val, pc);
	// asm (
	// 	"mov R0, #0xd0;"
	// 	"MSR CPSR, R0;"
	// 	"MRS R1, CPSR;"
	// 	"mov %0, R1;"
	// 	: "=r" ( val )        /* output */
	//  );
	// asm (
	// 	"mov %0, PC;"
	// 	: "=r" ( pc )
	// );
	// bwprintf(COM2, "%u %u\n\r", val, pc);
	// asm (
	// 	"swi 0;"
	// );
	// bwprintf(COM2, "returned from handle\n\r");


	return 0;
}

