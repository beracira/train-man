#include "td.h"
#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "nameserver.h"
#include "clockserver.h"
#include "common.h"
#include "irq.h"
#include "kernel.h"
#include "io.h"
#include "train_ui.h"
#include "terminal_input_handler.h"
#include "courier.h"
#include "sensors.h"
#include "track.h"

#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

unsigned int idle_ticks = 0;

void idle_task(void) {
  int * timerLoad = (int *)(TIMER1_BASE + LDR_OFFSET);
  int * volatile timerValue = (int *)(TIMER1_BASE + VAL_OFFSET);
  int * timerControl = (int *)(TIMER1_BASE + CRTL_OFFSET);

  // 508000 cycles/s = 50800 cycles/0.1s
  int cyclesPerTick = ONE_TICK / 40;

  // Load the timer
  // Disable first
  *timerControl = (*timerControl) ^ ENABLE_MASK;
  // Load initial value
  *timerLoad = cyclesPerTick;
  // Enable timer, periodic mode, 508 kHz clock
  *timerControl = ENABLE_MASK | MODE_MASK| CLKSEL_MASK ;
  volatile int i = 16000000;
  int j = 0;
  (void) i;
  (void) j;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile int * temp = &(ks->num_tasks);
  idle_ticks = 0;
  unsigned int pre = cyclesPerTick;
  while (*temp != 7) {
    unsigned int cur = *((int *)(timerValue));
    if (cur > pre) ++idle_ticks;
    pre = cur;
  }
  // idle_print();
  Exit();
}

void idle_print() {
  bwprintf(COM2, "\033[s\033[A\033[50Cidle usage: %d%%\n\r\033[u", idle_ticks * 100 / 40 / time_ticks);
}

void speed_test(void) {
  Putc(1, 10); // speed
  Putc(1, 71); // train num
 // set_train_speed(71, 10);
  Exit();
}

void track_test() {
  track_node * track = (track_node *) 0x01700000;
  int i = 0;
  int posn = 61;
  track_node *current = &track[posn];
  printf(2, "\033[A\033[2K\r\r\n\033[B");
 // printf(2, "\033[s\033[A\033[50C");
  printf(2, "Node:");
  for (i = 0; i < 20; i++) {
    printf(2, "%s, ", current->name);

    switch (current->type) {
      // case (NODE_NONE):
      //   break;
      // case (NODE_SENSOR):
      //   break;

      case (NODE_BRANCH):
        current = current->edge[current->dir].dest;
        break;

      // case (NODE_MERGE):
      //   current = current->edge[DIR_AHEAD].dest;
      //   break;
      // case (NODE_ENTER):
      //   break;
      // case (NODE_EXIT):
      //   break;
        
      default:
        current = current->edge[DIR_AHEAD].dest;
        break;
    }
  }
}

void firsttask(void) {

  // always init track data first
  init_data(TRACK_B);

  // K3 tasks
  Create(P_NAME_SERVER, nameserver);
  Create(P_CLOCK_SERVER, clockserver);
  Create(P_OTHER_SERVERS, IO_Server);
  Create(P_OTHER_SERVERS, courier_server);
  Create(P_OTHER_SERVERS, UI_Server);
  Create(P_MEDIUM, input_handle);
  Create(P_MEDIUM, get_sensor_data);
  Create(P_LOW, &idle_task);

  // don't want to run the track code until everything is initialized
  // note that this affect idle usage
  while (!(io_ready && ui_ready)) {
    Pass();
  }

  track_test();

  Exit();
}



