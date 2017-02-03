#ifndef CLOCKSERVER_H
#define CLOCKSERVER_H

#include "common.h"
extern unsigned int time_ticks;

#define CLK_DELAY 1
#define	CLK_TIME 2
#define CLK_DELAY_UNTIL 3
#define CLK_AWAIT_EVENT 4

#define CLK_TID 3

struct CLK_DELAY_LIST{
  int tid[MAX_TASKS];
  unsigned int wakeup_time[MAX_TASKS];
  int first;
  int last;
};


struct clk_request {
	int type;
  unsigned int ticks;
  int tid;
};

unsigned int Time(void);

int DelayUntil(unsigned int ticks);

int Delay(unsigned int ticks);

void remove_delay_list();

void clockserver(void);


#endif /* CLOCKSERVER_H */
