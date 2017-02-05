#ifndef SYSCALL_H
#define SYSCALL_H

#define TASK_DESCRIPTOR_START 0x01400000

#define KERNEL_STACK_START    0x01300000
#define USER_STACK_START 	    0x01200000
#define USER_STACK_SIZE       0x00015000

#define MAX_TASKS 50

int syscall(int code, int arg1, int arg2);

int kernel_Create(int priority, void (*code) ( ) );

int kernel_MyTid(void);

int kernel_MyParentTid(void);

int kernel_Pass(void);

int kernel_Exit(void);

int kernel_Send( int tid, void *msg, int msglen, void *reply, int rplen);

int kernel_kernel_Send( int tid, void *msg, int msglen, void *reply, int rplen);

int kernel_Receive( int *tid, void *msg, int msglen );

int kernel_Reply( int tid, void *reply, int replylen );

#endif /* SYSCALL_H */
