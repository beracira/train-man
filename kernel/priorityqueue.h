#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#define PRIORITY_QUEUE_START  0x01600000
#define QUEUE_SIZE  50

typedef enum {
  HIGH,
  MEDIUM,
  LOW
} priority_t;

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
int add_task_to_queue(int tid, priority_t p);
int remove_active_task_from_queue(priority_t p);
int schedule(void);

#endif /* PRIORITYQUEUE_H */
