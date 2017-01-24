#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#define CREATE      0
#define MYTID       1
#define MYPARENTPID 2
#define PASS        3
#define EXIT        4

int Create(int priority, void (*code) ( ) );

int MyTid(void);

int MyParentTid(void);

void Pass(void);

void Exit(void);

#endif // USER_SYSCALL_H
