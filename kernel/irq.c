#include "irq.h"
#include "../io/include/ts7200.h"

void irq_enable_timer(void) {
  int * int_enable = (int *) (VIC2_BASE + VICxIntEnable);
  *int_enable = *int_enable | (1 << (51 - 32));
}

void irq_clear_timer(void) {
  int * clear = (int *) (TIMER3_BASE + CLR_OFFSET);
  *clear = 1;
}

void irq_disable_timer(void) {
  // int * int_enable = (int *) (VIC2_BASE + VICxIntEnable);
  // *int_enable = *int_enable  & ~(1 << (51 - 32));

  int * int_en_clean = (int *) (VIC2_BASE + VICxIntEnClear);
  *int_en_clean = *int_en_clean | (1 << (51 - 32));
}
