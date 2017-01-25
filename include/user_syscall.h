#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#define CREATE      1
#define MYTID       2
#define MYPARENTPID 3
#define PASS        4
#define EXIT        5

int Create(int priority, void (*code) ( ) );

int MyTid(void);

int MyParentTid(void);

void Pass(void);

void Exit(void);

#endif // USER_SYSCALL_H
