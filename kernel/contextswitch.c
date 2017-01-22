#include "contextswitch.h"
#include "functions.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void syscall(int code) {
  // 
  asm("mov	ip, sp;");
	asm("stmfd	sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr};");
	asm("swi 0;");
  //   int *flags, *data;
	// 	flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
	// 	data = (int *)( UART2_BASE + UART_DATA_OFFSET );
	// while( ( *flags & TXFF_MASK ) ) ;
	// *data = 's';
	asm("ldmfd	sp, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc};");
  
  // bwprintf(COM2, "syscall end \n\r");
}

void exit_kernel(void){
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("ldr r8, [r9, #12];");    // load spsr
  asm("msr spsr, r8;");         // change spsr
  asm("ldr lr, [r9, #4];");     // load lr
  asm("ldr r0, [r9, #8];");     // load return value to r0
  asm("msr CPSR_c, #0xdf;");    // switch to system mode
  asm("ldr sp, [r9, #0];");     // load sp
  asm("msr CPSR_c, #0xd3;");    // back to supervisor mode
  asm("movs pc, lr;");          // let it go
}

void swi_handler(void){
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("msr CPSR_c, #0xDF;");      // switch to system mode
  asm("str sp, [r9, #0];");     // store sp
  asm("str lr, [r9, #4];");     // save lr to stack
  asm("msr CPSR_c, #0xD3;");      // back to svc
  asm("mrs r8, spsr;");         // move spsr to r8
  asm("str r8, [r9, #12];");    // store spsr
  //asm("mov sp, r9;");           // move kernel stack pointer to sp
  asm("ldr r0, [lr,#-4];");     // load swi code 
  asm("bic r0, r0, #0xff000000;");  // get number
  asm("bl handle;");

  int *flags, *data;
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
	while( ( *flags & TXFF_MASK ) ) ;
	*data = 'c';
}
