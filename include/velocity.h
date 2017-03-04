#ifndef VELOCITY_H
#define VELOCITY_H

#define SENSOR_ACTIVATED 1



struct vel_request {
  int type;
  int sensor;
};

struct train_velocity {
  int length;
  int num;
  int time;
};

void sensor_data_received(int i);
void velocity_server();

#endif /* VELOCITY_H */
