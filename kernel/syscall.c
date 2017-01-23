#include "syscall.h"
#include "functions.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void syscall(int code) {
  // 
  asm("mov	ip, sp;");
	asm("stmfd	sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
	asm("swi 0;");
	//asm("ldmfd	sp, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc};"); // loads usr state
  
  // bwprintf(COM2, "syscall end \n\r");
}
