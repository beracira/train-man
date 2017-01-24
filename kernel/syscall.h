#ifndef SYSCALL_H
#define SYSCALL_H

#include "priorityqueue.h"

#define TASK_DESCRIPTOR_START 0x01400000

#define KERNEL_STACK_START    0x01300000
#define USER_STACK_START 	    0x01200000
#define USER_STACK_SIZE       0x00015000

#define MAX_TASKS 16

int syscall(int code, int arg1, int arg2);

int kernel_Create(priority_t priority, void (*code) ( ) );

int kernel_MyTid(void);

int kernel_MyParentTid(void);

void kernel_Pass(void);

void kernel_Exit(void);

#endif /* SYSCALL_H */
