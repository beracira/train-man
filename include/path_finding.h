#ifndef PATH_FINDING_H
#define PATH_FINDING_H

#define MAX_PATH_LENGTH 100

#define train_76 0
#define train_71 1
#define train_64 2

extern int target_sensor;

extern int path[MAX_PATH_LENGTH];
extern int path_len;
extern int train_velocity[5][15][80][80];
extern double default_speed[5][15];
int find_path(int train_number, int origin, int dest, int dist_init);
void train_velocity_init();
void velocity_print(int train_number, int speed);
int get_sensor_color(int sensor);

#endif // PATH_FINDING_H
