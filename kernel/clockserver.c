#include "clockserver.h"
#include "user_syscall.h"
#include "common.h"
#include "../io/include/bwio.h"

struct CLK_DELAY_LIST * clk_delay_list_ptr = 0;

unsigned int time_ticks = 0;

unsigned int Time(void) {
  struct clk_request input;
  struct clk_request output;

  input.type = CLK_TIME;
  input.ticks = -1;
  input.tid = -1;

  // double check tid
  Send(CLK_TID, &input, sizeof(struct clk_request), &output, sizeof(struct clk_request));

  return output.ticks;
}

int DelayUntil(unsigned int ticks) {
  struct clk_request input;
  struct clk_request output;

  input.type = CLK_DELAY_UNTIL;
  input.ticks = ticks;
  input.tid = -1;

  // double check tid
  Send(CLK_TID, &input, sizeof(struct clk_request), &output, sizeof(struct clk_request));
  int sender_tid = 0, dummy1 = 1, dummy2 = 2;

  Receive( &sender_tid, &dummy1, sizeof(int));
  Reply(sender_tid, &dummy2, sizeof(int));

  return output.ticks;
}

int Delay(unsigned int ticks) {
  DelayUntil(time_ticks + ticks);
}

void insert_delay_list(int tid, unsigned int ticks) {
  int i = 0;
  for (i = 0; i < clk_delay_list_ptr->last; ++i) {
    if (clk_delay_list_ptr->wakeup_time[i] > ticks) break;
  }

  int j;
  for (j = clk_delay_list_ptr->last; j > i; --j) {
    clk_delay_list_ptr->tid[j] = clk_delay_list_ptr->tid[j - 1];
    clk_delay_list_ptr->wakeup_time[j] = clk_delay_list_ptr->wakeup_time[j - 1];
  }
  clk_delay_list_ptr->tid[i] = tid;
  clk_delay_list_ptr->wakeup_time[i] = ticks;
  ++clk_delay_list_ptr->last;
}

void remove_delay_list() {
  while (clk_delay_list_ptr->last != 0 && clk_delay_list_ptr->wakeup_time[0] < time_ticks) {
    int tid = clk_delay_list_ptr->tid[0];
    int dummy1 = 1, dummy2 = 2;
    kernel_kernel_Send(tid, &dummy1, sizeof(int), &dummy2, sizeof(int));
    int i;
    for (i = 0; i < clk_delay_list_ptr->last; ++i) { // last one should be -1
      clk_delay_list_ptr->tid[i] = clk_delay_list_ptr->tid[i + 1];
      clk_delay_list_ptr->wakeup_time[i] = clk_delay_list_ptr->wakeup_time[i + 1];
    }
    --clk_delay_list_ptr->last;
  }
}

void init_delay_list(void) {
  clk_delay_list_ptr->first = 0;
  clk_delay_list_ptr->last = 0;

  int i = 0;
  for (i = 0; i < MAX_TASKS ; ++i) {
    clk_delay_list_ptr->tid[i] = -1;
    clk_delay_list_ptr->wakeup_time[i] = 0;
  }
}

void clockserver(void) {

  struct CLK_DELAY_LIST clk_delay_list;
  clk_delay_list_ptr = &clk_delay_list;
  init_delay_list(); 

  time_ticks = 0;
  timer_init();

  int sender_tid = -1;
  struct clk_request req;
  struct clk_request result;
 
	while (1 + 1 ==2) {
		Receive( &sender_tid, &req, sizeof(struct clk_request));

    switch(req.type) {
      case CLK_TIME:
        result.type = CLK_TIME;
        result.ticks = time_ticks;
        result.tid = sender_tid;
        break;

      case CLK_DELAY_UNTIL:
        insert_delay_list(sender_tid, req.ticks);
        break;

      default:
        break;
    }

    Reply(sender_tid, &result, sizeof(struct clk_request));

	}
}

