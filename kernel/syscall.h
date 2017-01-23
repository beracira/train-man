#ifndef CONTEXTSWITCH_H
#define CONTEXTSWITCH_H

#define TASK_DESCRIPTOR_START 0x01400000

#define KERNEL_STACK_START    0x01300000
#define USER_STACK_START 	    0x01200000
#define USER_STACK_SIZE       0x00015000

#define MAX_TASKS 16

void syscall(int code);

void handle(int code);

void exit_kernel(void);

void initialize(void);

void swi_handler(void);

#endif /* CONTEXTSWITCH_H */
