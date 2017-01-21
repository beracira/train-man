#include "contextswitch.h"
#include "functions.h"

void exit_kernel(void){

  asm("msr SPSR, #0xd0;");
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("ldr ip, [r9, #0];");     // load sp
  asm("ldr r1, [r9, #4];");     // load lr in r1
  asm("mov lr, r1;");           // move to lr
  asm("ldr r1, [r9, #8];");     // load ret to r1
  asm("mov r0, r1;");
  asm("msr CPSR_c, #0xdf;"); // switch to system mode
  asm("mov sp, ip;");
  asm("msr CPSR_c, #0xd3;"); // back to supervisor mode
  asm("msr SPSR, #0xd0;");
  asm("movs pc, lr;"); // let it go
}

