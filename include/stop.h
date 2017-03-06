#ifndef STOP_H
#define STOP_H

void stop_worker();
void Stop(int train_number, int sensor, int time_wait);
void remove_from_stop_queue(int last_sensor);

#endif // STOP_H
