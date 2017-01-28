#ifndef MESSAGES_H
#define MESSAGES_H

int Send( int tid, void *msg, int msglen, void *reply, int rplen);

int Receive( int *tid, void *msg, int msglen );

int Reply( int tid, void *reply, int replylen );

#endif /* MESSAGES_H */
