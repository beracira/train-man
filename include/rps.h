#ifndef RPS_H
#define RPS_H

#define RPS_SIGN_UP 0
#define RPS_PLAY 1
#define RPS_QUIT 2

#define RPS_ROCK 1
#define RPS_PAPER 2
#define RPS_SCISSORS 3

struct rps_request {
  int type;
  int move;
  int tid;
};

struct rps_server{
  int move;
  int tid;
};

int rps_sign_up(void);

int rps_play(int move);

int rps_quit(void);

void rps_server(void);

#endif /* RPS_H */
