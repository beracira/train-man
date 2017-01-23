#include "functions.h"
#include "syscall.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void firsttask(void) {
 // Create(2, &foo);
  bwprintf(COM2, "first task\n\r");
  syscall(0);
  bwprintf(COM2, "first task end\n\r");
  syscall(0);
  bwprintf(COM2, "first task end2\n\r");
  syscall(0);
  bwprintf(COM2, "first task end3\n\r");
  syscall(0);
  bwprintf(COM2, "first task end4\n\r");
}

void foo(void){
  bwprintf(COM2, "please work \n\r");
}
