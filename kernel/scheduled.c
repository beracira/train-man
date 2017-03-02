#include "scheduled.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "clockserver.h"
#include "courier.h"
#include "io.h"
#include "train_ui.h"

void scheduled_tasks() {
  // Create(P_NOTIFIER, wake_trains);
  Create(P_NOTIFIER, send_sensor_request);

  Exit();
}


void wake_trains() {
  while (1) {
    Delay(10);
    if (courier_ready) wake_train();
  }
}

void send_sensor_request() {
  struct cr_request input;
  struct cr_request output;
  input.type = CR_SENSOR_REQUEST;

  while (1) {
    Delay(50);
    if (io_ready && ui_ready && courier_ready) {
      // printf(2, "before scheduled send %d\n\r", input.type);
      Send(CR_TID, &input, sizeof(input), &output, sizeof(output));
      // printf(2, "after scheduled send %d\n\r", input.type);
    }
  }
  Exit();
}
