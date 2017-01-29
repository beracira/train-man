#include "priorityqueue.h"
#include "td.h"

struct priority_queue * q = (struct priority_queue *) PRIORITY_QUEUE_START;

void init_queue(void) {
  int i;
  int j;

  // initiate everything to -1
  for (j = 0; j < NUM_QUEUES; j++) {
    q[j].first = 0;
    q[j].last = 0;
    q[j].priority = j;
    for (i = 0; i < QUEUE_SIZE; i++) {
      q[j].tid[i] = -1;
    }
  }

  // first task has tid 0 and medium priority
  q[P_MEDIUM].tid[0] = 1;
  q[P_MEDIUM].first = 0;
  q[P_MEDIUM].last = 1;
}

// adds a task to the end of a queue
int add_task_to_queue(int tid, int p) {
  if (p >= NUM_QUEUES) {
    // invalid priority
    return -1;
  }

  int t;
  t = q[p].tid[q[p].last];
  if (t == -1) {
    q[p].tid[q[p].last] = tid;
    q[p].last = (q[p].last + 1) % QUEUE_SIZE;
    return tid;
  }

  // overflow
  return 0;
}

// removes first task in queue
// return tid of the task removed
int remove_active_task_from_queue(int tid, int p) {

  if (p >= NUM_QUEUES) {
    // invalid priority
    return -1;
  }

  int t;
  t = q[p].tid[q[p].first];
  if (t == tid) {
    q[p].tid[q[p].first] = -1;
    q[p].first = (q[p].first + 1) % QUEUE_SIZE;
    return t;
  }

  // nothing to remove
  return 0;

}

// returns tid of task to be run
int schedule(void) {
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  int j;
  int i = q[j].tid[q[j].first];
  int a = q[j].first;
  int b = q[j].last;
  for (j = 0; j < NUM_QUEUES; j++) {
    for (i = a; i < b; i++) {
      int tid = q[j].tid[i];
      if (td[tid].state != READY) {
        reschedule(tid, j);
      }
      else {
        return q[j].tid[i];
      }
    }
  }

  // nothing to schedule
  return -1;
}

// wrapper for remove and add
int reschedule(int tid, int p) {
  remove_active_task_from_queue(tid, p);
  return add_task_to_queue(tid, p);
}

