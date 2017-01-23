#include "syscall.h"
#include "functions.h"
#include "td.h"
#include "priorityqueue.h"
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

// Create allocates and initializes a task descriptor, using the given priority, and the given
// function pointer as a pointer to the entry point of executable code, essentially a function
// with no arguments and no return value. 
// When Create returns the task descriptor has all the state needed to run the task, the 
// task’s stack has been suitably initialized, and the task has been entered into its ready 
// queue so that it will run the next time it is scheduled.
// Returns:
// tid
// -1 if the priority is invalid.
// -2 if the kernel is out of task descriptors.
int Create( int priority, void (*code) ( ) ) {
  // create td with given priority and code
  // add to queue after a tid has been assigned to the new task

}

// Returns tid, the positive integer task id of the task that calls it.
int MyTid(void){

}

// MyParentTid returns the task id of the task that created the calling task. This will be
// problematic only if the task has exited or been destroyed, in which case the return
// value is implementation-dependent.
int MyParentTid(void){

}

// Pass causes a task to stop executing. The task is moved to the end of its priority queue,
// and will resume executing when next scheduled.
void Pass(void){

}

// Exit causes a task to cease execution permanently. It is removed from all priority
// queues, send queues, receive queues and awaitEvent queues. Resources owned by the
// task, primarily its memory and task descriptor are not reclaimed.
void Exit(void){

}