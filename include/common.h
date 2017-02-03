#ifndef COMMON_H
#define COMMON_H

#define SEC 50800
#define TIMER3_BASE 0x80810080
#define TIMER3_CONTROL 0x80810088
#define LDR_OFFSET  0x00000000  // 16/32 bits, RW
#define VAL_OFFSET  0x00000004  // 16/32 bits, RO
#define CRTL_OFFSET 0x00000008  // 3 bits, RW
#define CLR_OFFSET  0x0000000c  // anything W
#define TIME_PTR 0x80810084

int timer_init();
void srand();
unsigned int rand();

int strlen( const char * const str);

void strcpy( char * const origin, char * const dest);

int strcmp( const char * const str1, const char * const str2);

#endif /* COMMON_H */
