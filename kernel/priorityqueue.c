#include "priorityqueue.h"

struct priority_queue * q = (struct priority_queue *) PRIORITY_QUEUE_START;

void init_queue(void) {
  int i;

  // initiate everything to -1
  for (i = 0; i < QUEUE_SIZE; i++) {
    q->high.tid[i] = -1;
    q->mid.tid[i] = -1;
    q->low.tid[i] = -1;
  }

  // first task has tid 0 and medium priority
  q->mid.tid[0] = 1;
  q->mid.first = 0;
  q->mid.last = 1;

  // other priorities
  q->high.first = 0;
  q->high.last = 0;
  q->low.first = 0;
  q->low.last = 0;
}

// adds a task to the end of a queue
int add_task_to_queue(int tid, priority_t p) {
  int t;
  if (p == HIGH) {
    t = q->high.tid[q->high.last];
    if (t == -1) {
      q->high.tid[q->high.last] = tid;
      q->high.last = (q->high.last + 1) % QUEUE_SIZE;
      return 1;
    }
  }
  if (p == MEDIUM) {
    t = q->mid.tid[q->mid.last];
    if (t == -1) {
      q->mid.tid[q->mid.last] = tid;
      q->mid.last = (q->mid.last + 1) % QUEUE_SIZE;
      return 1;
    }
  }
  if (p == LOW) {
    t = q->low.tid[q->low.last];
    if (t == -1) {
      q->low.tid[q->low.last] = tid;
      q->low.last = (q->low.last + 1) % QUEUE_SIZE;
      return 1;
    }
  }
  
  return 0;
}

// removes first task in queue
// return tid of the task removed
int remove_active_task_from_queue(int tid, priority_t p) {
  int t = -1;
  if (p == HIGH) {
    t = q->high.tid[q->high.first];
    if (t == tid) {
      q->high.tid[q->high.first] = -1;
      q->high.first = (q->high.first + 1) % QUEUE_SIZE;
      return t;
    }
  }
  else if (p == MEDIUM) {
    t = q->mid.tid[q->mid.first];
    if (t == tid) {
      q->mid.tid[q->mid.first] = -1;
      q->mid.first = (q->mid.first + 1) % QUEUE_SIZE;
      return t;
    }
  }
  else if (p == LOW) {
    t = q->low.tid[q->low.first];
    if (t == tid) {
      q->low.tid[q->low.first] = -1;
      q->low.first = (q->low.first + 1) % QUEUE_SIZE;
      return t;
    }
  }
  else {
    return 0;
  }
  return t;
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

// wrapper for remove and add
void reschedule(int tid, priority_t p) {
  remove_active_task_from_queue(tid, p);
  add_task_to_queue(tid, p);
}

