#ifndef SENSORS_H
#define SENSORS_H
#include "track.h"

extern int last_sensor;
extern int sensor_requested;
extern int time_to_next_sensor;

void get_sensor_data();

track_node * get_next_sensor(int sensor);

int get_next_sensor_dist(int sensor);

void predict_path(int sensor);

int update_train_state(int sensor);

void train_init();

#endif // SENSORS_H
