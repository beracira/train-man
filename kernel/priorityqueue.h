#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#define PRIORITY_QUEUE_START  0x01600000
#define QUEUE_SIZE  50

//priorities
#define P_HIGH 0
#define P_MEDIUM 1
#define P_LOW 2

struct queue {
  int tid[QUEUE_SIZE];
  int first;
  int last;
};

struct priority_queue {
  struct queue high;
  struct queue mid;
  struct queue low;
};

void init_queue(void);
int add_task_to_queue(int tid, int p);
int remove_active_task_from_queue(int tid, int p);
int schedule(void);
int reschedule(int tid, int p);

#endif /* PRIORITYQUEUE_H */
