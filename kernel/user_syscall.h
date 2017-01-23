#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

int Create(priority_t priority, void (*code) ( ) );

int MyTid(void);

int MyParentTid(void);

void Pass(void);

void Exit(void);

#endif // USER_SYSCALL_H
