#ifndef SENSORS_H
#define SENSORS_H
#include "track.h"

extern int last_sensor;
extern int sensor_requested;
extern int time_to_next_sensor;
extern int RUNNING_TRAIN;

void get_sensor_data();

track_node * get_next_sensor(int sensor);

int get_next_sensor_dist(int sensor);

void predict_path(int sensor);

int update_train_state(int client, int sensor);

void train_init();

void the_evil_guy();

#endif // SENSORS_H
