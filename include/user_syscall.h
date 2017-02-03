#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#define IRQ         0
#define CREATE      1
#define MYTID       2
#define MYPARENTPID 3
#define PASS        4
#define EXIT        5
#define SEND        6
#define RECEIVE     7
#define REPLY       8

int Create(int priority, void (*code) ( ) );

int MyTid(void);

int MyParentTid(void);

void Pass(void);

void Exit(void);

int Send( int tid, void *msg, int msglen, void *reply, int rplen);

int Receive( int *tid, void *msg, int msglen );

int Reply( int tid, void *reply, int replylen );

#endif // USER_SYSCALL_H
