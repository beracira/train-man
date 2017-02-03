#include "irq.h"
#include "../io/include/ts7200.h"

void irq_enable_timer(void) {
  int * enable_timer = (int *) (VIC2_BASE + VICxIntEnable);
  *enable_timer = *enable_timer  | (1 << (51 - 32));
}

void irq_disable_timer(void) {
  int * clear = (int *) (TIMER3_BASE + CLR_OFFSET);
  *clear = 1;
}
