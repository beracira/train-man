#ifndef COMMON_H
#define COMMON_H

#define SEC 508000
#define TIMER3_BASE 0x80810080
#define TIMER3_CONTROL 0x80810088
#define LDR_OFFSET  0x00000000  // 16/32 bits, RW
#define VAL_OFFSET  0x00000004  // 16/32 bits, RO
#define CRTL_OFFSET 0x00000008  // 3 bits, RW
#define TIME_PTR 0x80810084

int timer_init();

int strlen( char * str );

void strcpy( char * origin, char * dest);

int strcmp( char * str1, char * str2);

#endif /* COMMON_H */
