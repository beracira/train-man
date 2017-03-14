#ifndef STOP_H
#define STOP_H

void stop_worker();
void Stop(int train_number, int sensor, int time_wait);
void remove_from_stop_queue();
void cancel_stop(int train_number);

#endif // STOP_H
