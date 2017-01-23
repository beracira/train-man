#ifndef SYSCALL_H
#define SYSCALL_H

#define TASK_DESCRIPTOR_START 0x01400000

#define KERNEL_STACK_START    0x01300000
#define USER_STACK_START 	    0x01200000
#define USER_STACK_SIZE       0x00015000

#define MAX_TASKS 16

void syscall(int code);

int Create( int priority, void (*code)( ));

int MyTid(void);

int MyParentTid(void);

void Pass(void);

void Exit(void);

#endif /* SYSCALL_H */
