#ifndef PATH_FINDING_H
#define PATH_FINDING_H

#define MAX_PATH_LENGTH 100

#define train_76 0
#define train_71 1
#define train_64 2
#define train_63 3

struct Train {
  int prev_sensor;
  int cur_sensor;
  int speed;
  int missed_count;
  int direction;
  int time_current_sensor;
  int predict_sensors[100];
};

extern int target_sensor;

extern struct Train train_64_struct;
extern int path[MAX_PATH_LENGTH];
extern int path_len;
extern int train_velocity[5][15][80][80];
extern double default_speed[5][15];
int find_path(int train_number, int origin, int dest, int dist_init);
void train_velocity_init();
void velocity_print(int train_number, int speed);
int get_sensor_color(int sensor);
void short_move(int train_number, int dist);
void update_train_velocity(int train_number, int speed, int start, int end, int new_time);

#endif // PATH_FINDING_H
