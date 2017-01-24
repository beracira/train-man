#include "functions.h"
#include "user_syscall.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void firsttask(void) {
 // Create(2, &foo);
  // bwprintf(COM2, "first task\n\r");
  // int i = syscall(0,1,2);
  // bwprintf(COM2, "first1 task end: %d\n\r", i);
  // i = syscall(123,456,789);
  // bwprintf(COM2, "first2 task end: %d\n\r", i);
  // i = syscall(987,654,321);
  // bwprintf(COM2, "first3 task end: %d\n\r", i);
  // i = syscall(111,111,111);
  // bwprintf(COM2, "first4 task end: %d\n\r", i);

  bwprintf(COM2, "first task\n\r");
  Pass();
  bwprintf(COM2, "first1 task end\n\r");
  Pass();
  bwprintf(COM2, "first2 task end\n\r");
  Pass();
  bwprintf(COM2, "first3 task end:\n\r");
  Pass();
  bwprintf(COM2, "first4 task end:\n\r");
}

void foo(void){
  bwprintf(COM2, "please work \n\r");
}
