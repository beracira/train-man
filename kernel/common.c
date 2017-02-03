#include "common.h"
#include "../io/include/ts7200.h"

int timer_init() {
  // *((int *) (TIMER3_CONTROL)) =  0x00000048;
  // *((int *) (TIMER3_CONTROL + CLR_OFFSET)) = 1;
  // *((int *) (TIMER3_BASE + LDR_OFFSET)) = (SEC);
  // *((int *) (TIMER3_CONTROL)) =  0x000000c8;


  int * timerLoad = (int *)(TIMER3_BASE + LDR_OFFSET);
  int * timerValue = (int *)(TIMER3_BASE + VAL_OFFSET);
  int * timerControl = (int *)(TIMER3_BASE + CRTL_OFFSET);

  // 508000 cycles/s = 50800 cycles/0.1s
  int cyclesPerTick = 50800;

  // Load the timer
  // Disable first
  *timerControl = (*timerControl) ^ ENABLE_MASK;
  // Load initial value
  *timerLoad = cyclesPerTick;
  // Enable timer, periodic mode, 508 kHz clock
  *timerControl = ENABLE_MASK | MODE_MASK| CLKSEL_MASK ;


  return 0;
}

void srand() {
  timer_init();
}

unsigned int rand() {
  unsigned int cur = *((int *)(TIME_PTR));
  return cur;
}

int strlen( const char * const str) {
  int i = 0;
  while (str[i++]);
  return i;
}

void strcpy( char * const origin, char * const dest) {
  int i = 0;
  while ((dest[i] = origin[i++]));
  return; 
}

int strcmp( const char * const str1, const char * const str2) {
  int i = 0;
  int len_1 = strlen(str1);
  int len_2 = strlen(str2);
  if (len_1 != len_2) return 0;
  while (i < len_1) 
    if (str1[i] != str2[i++]) return 0;
  return 1;
}
