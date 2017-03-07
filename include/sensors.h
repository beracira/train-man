#ifndef SENSORS_H
#define SENSORS_H
#include "track.h"

extern int last_sensor;
extern int sensor_requested;

void get_sensor_data();

track_node * get_next_sensor(int sensor);

int get_next_sensor_dist(int sensor);


#endif // SENSORS_H
