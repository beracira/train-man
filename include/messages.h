#ifndef MESSAGES_H
#define MESSAGES_H


int Send(int tid, char *msg, int msglen, char *reply, int rplen);

int Receive( int *tid, char *msg, int msglen );

int Reply( int tid, char *reply, int rplen );

#endif /* MESSAGES_H */
