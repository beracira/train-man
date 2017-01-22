#include "priorityqueue.h"

struct priority_queue * q = (struct priority_queue *) PRIORITY_QUEUE_START;

void init_queue(void) {
  int i;

  for (i = 0; i < QUEUE_SIZE; i++) {
    q->high.tid[i] = -1;
    q->mid.tid[i] = -1;
    q->low.tid[i] = -1;
  }
  q->mid.tid[0] = 0;

  q->high.first = 0;
  q->high.last = 0;
  q->mid.first = 0;
  q->mid.last = 1;
  q->low.first = 0;
  q->low.last = 0;

}

int add_task_to_queue(int tid, priority_t p) {
  // todo: overflow
  if (p == HIGH) {
    q->high.tid[q->high.last] = tid;
    q->high.last = (q->high.last + 1) % QUEUE_SIZE;
    return 1;
  }
  else if (p == MEDIUM) {
    q->mid.tid[q->mid.last] = tid;
    q->mid.last = (q->mid.last + 1) % QUEUE_SIZE;
    return 1;
  }
  else if (p == LOW) {
    q->mid.tid[q->mid.last] = tid;
    q->mid.last = (q->mid.last + 1) % QUEUE_SIZE;
    return 1;
  }
  else {
    return 0;
  }
}

// removes first task in queue
int remove_active_task_from_queue(priority_t p) {
  if (p == HIGH) {
    q->high.tid[q->high.first] = -1;
    q->high.first = (q->high.first + 1) % QUEUE_SIZE;
    return 1;
  }
  else if (p == MEDIUM) {
    q->mid.tid[q->mid.first] = -1;
    q->mid.first = (q->mid.first + 1) % QUEUE_SIZE;
    return 1;
  }
  else if (p == LOW) {
    q->mid.tid[q->mid.first] = -1;
    q->mid.first = (q->mid.first + 1) % QUEUE_SIZE;
    return 1;
  }
  else {
    return 0;
  }
}

// returns tid of task to be run
int schedule(void) {
  if (q->high.tid[q->high.first] != -1) {
    return q->high.tid[q->high.first];
  }
  else if (q->mid.tid[q->mid.first] != -1) {
    return q->mid.tid[q->mid.first];
  }
  else if (q->low.tid[q->low.first] != -1) {
    return q->low.tid[q->low.first];
  }
  else {
    return -1;
  }
}
