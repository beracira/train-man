#include "syscall.h"
#include "functions.h"
#include "td.h"
#include "priorityqueue.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

int syscall(int code, int arg1, int arg2) {
  (void) code;
  (void) arg1;
  (void) arg2;
  asm("mov	ip, sp;");
	asm("stmfd	sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
	asm("swi 0;");
 // bwprintf(COM2, "asfaewufawjioef\n\r");

  return 0;
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
int kernel_Create(int priority, void (*code) ( ) ) {
  int tid;

  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  int parent_tid = ks->tid;

  // create td with given priority and code 
  tid = td_add(code, priority, parent_tid);

  // add to priority queue
  add_task_to_queue(tid, priority); // returns -1 if invalid priority, tid if okay

  reschedule(ks->tid, ks->priority);

  return tid;
}

// Returns tid, the positive integer task id of the task that calls it.
int kernel_MyTid(void){
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  int tid = ks->tid;

  reschedule(ks->tid, ks->priority);

  return tid;
}

// MyParentTid returns the task id of the task that created the calling task. This will be
// problematic only if the task has exited or been destroyed, in which case the return
// value is implementation-dependent.
int kernel_MyParentTid(void){
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  int tid = ks->parent_tid;

  reschedule(ks->tid, ks->priority);

  return tid;
}

// Pass causes a task to stop executing. The task is moved to the end of its priority queue,
// and will resume executing when next scheduled.
int kernel_Pass(void){
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  int tid = ks->tid;
  int priority = ks->priority;
  
  // need to get tid and priority of the task that called

  return reschedule(tid, priority);
}

// Exit causes a task to cease execution permanently. It is removed from all priority
// queues, send queues, receive queues and awaitEvent queues. Resources owned by the
// task, primarily its memory and task descriptor are not reclaimed.
int kernel_Exit(void){
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  (void) td;
  int tid = ks->tid;
  int priority = ks->priority;

  // need to get tid and priority of the task that called

  return remove_active_task_from_queue(tid, priority);
}


/*
Send sends a message to another task and receives a reply. 

msg: The message, in a buffer in the sending task’s address space is copied to the address space of the task to which it is sent by the kernel. 
reply: Send supplies a buffer into which the reply is to be copied, and the size of the buffer so that the kernel can detect overflow. 

When Send returns without error it is guaranteed that the message has been received, and that a reply has been sent, not necessarily by the same task. 
If either the message or the reply is a string it is necessary that the length should include the terminating null.

The kernel will not overflow the reply buffer. The caller is expected to compare the return value to the size of the reply buffer. 
If part of the reply is missing the return value will exceed the size of the supplied reply buffer.

There is no guarantee that Send will return. 
If, for example, the task to which the message is directed never calls Receive, Send never returns and the sending task remains blocked forever.

Send has a passing resemblance, and no more, to remote procedure call.

Returns
>-1 The size of the message responded by the replying task. The message is less than
or equal to the size of the buffer provided for it. Longer responses are truncated.
-1 The reply message was truncated.
-2 The task id supplied is not the task id of an existing task.
-3 The send-receive-reply transaction could not be completed. - eg sender == receiver
*/
int kernel_Send( int tid, void *msg, int msglen, void *reply, int rplen) {

  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  // TODO: check if td exists

  int s_tid = ks->tid;

  if (td[tid].state == SEND_BLOCKED) {
    td[tid].state = READY; 
  }

  int tail = td[tid].sendq_last;
  td[tid].sendq[tail].sender_tid = s_tid;
  td[tid].sendq[tail].msg = msg;
  td[tid].sendq[tail].msglen = msglen;
  td[tid].sendq[tail].reply = reply;
  td[tid].sendq[tail].rplen = rplen;
  td[tid].sendq_last = (td[tid].sendq_last + 1) % MAX_TASKS; // assume no overflow
  td[s_tid].state = RECEIVE_BLOCKED;

  reschedule(ks->tid, ks->priority);
  return 0;
}

/*
Receive blocks until a message is sent to the caller, then returns with the message in its message buffer and tid set to the task id of the task that sent the message. 
Messages sent before Receive is called are retained in a send queue, from which they are received in first-come, first-served order.

msg: The argument msg must point to a buffer at least as large as msglen. 
If the size of the message received exceeds msglen, no overflow occurs and the buffer will contain the first msglen characters of the message sent.

The caller is expected to compare the return value, which contains the size of the message that was sent, to determine whether or not the message is complete, and to
act accordingly.

Returns
>-1 The size of the message received, which is less than or equal to the size of the
message buffer supplied. Longer messages are truncated.
-1 The message is truncated.
*/
int kernel_Receive( int *tid, void *msg, int msglen ) {

  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  int my_tid = ks->tid;

  int first = td[my_tid].sendq_first;

  int retval = -2;

  if (first != td[my_tid].sendq_last) { // not empty
    int i;

    char * origin = (char *) td[my_tid].sendq[first].msg;
    char * dest = (char *) msg;

    *tid  = td[my_tid].sendq[first].sender_tid;

    //td[my_tid].sendq_first = (td[my_tid].sendq_first + 1) % MAX_TASKS;

    if (td[my_tid].sendq[first].msglen <= msglen) { // origin msg <= dest msg
      for (i = 0; i < td[my_tid].sendq[first].msglen; i++) {
        dest[i] = origin[i];
      }
      retval = td[my_tid].sendq[first].msglen;
    }

    else {
      for (i = 0; i < msglen; i++) {
        dest[i] = origin[i];
      }
      retval = -1;
    }

    td[td[my_tid].sendq[first].sender_tid].state = REPLY_BLOCKED;
  }

  else { 
    // not very possible 
    retval = -2;
  }

  reschedule(ks->tid, ks->priority);
  return retval;
}

/*
Reply sends a reply to a task that previously sent a message. 
When it returns without error, the reply has been copied into the sender’s address space. 
The calling task and the sender return at the same logical time, so whichever is of higher priority runs first.
If they are of the same priority the sender runs first.

Returns
0 The reply succeeded.
-1 The message was truncated.
-2 The task id is not the task id of an existing task.
-3 The task id is not the task id of a reply blocked task.
*/
int kernel_Reply( int tid, void *reply, int replylen ) {

  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  int retval = 0; // TODO: error checking
  int my_tid = ks->tid;

  int first = (int)td[my_tid].sendq_first;
  struct Sender * sender = (struct Sender *) &(td[my_tid].sendq[first]);

  char * origin = (char *) reply;
  char * dest = (char *) sender->reply;
  if (sender->rplen < replylen) {
    int i;
    for (i = 0; i < sender->rplen; ++i) {
      dest[i] = origin[i];
    }
    retval = -1;
    td[tid].return_value = -1;

  } else {
    int i;
    for (i = 0; i < replylen; ++i) {
      dest[i] = origin[i];
    }
    retval = 0;
    td[tid].return_value = replylen;
  }

  td[my_tid].state = READY;
  td[tid].state = READY;
  // error: tid not first sendq tid

  td[my_tid].sendq_first = (td[my_tid].sendq_first + 1) % MAX_TASKS;
  reschedule(ks->tid, ks->priority);
  return retval;
}
