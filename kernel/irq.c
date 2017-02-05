#include "irq.h"
#include "../io/include/ts7200.h"
#include "clockserver.h"
#include "user_syscall.h"

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

void timer_notifier(void) {
  int my_tid = MyTid();
  await_event_list_ptr[TIMER_EVENT] = my_tid;
  struct clk_request req;
  struct clk_request result;
  req.tid = my_tid;

  while (1 + 1 == 2) {
    AwaitEvent(TIMER_EVENT);
    req.type = CLK_INC;
    req.ticks = 0;
    Send(CLK_TID, &req, sizeof(struct clk_request), &result, 0);
  }
}
