#include "functions.h"
#include "contextswitch.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void firsttask(void) {
 // Create(2, &foo);
  bwprintf(COM2, "first task\n\r");
  syscall(0); 
}

void foo(void){
  bwprintf(COM2, "please work \n\r");
}