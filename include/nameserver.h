#ifndef NAMESERVER_H
#define NAMESERVER_H

#define NAME_SERVER_START 0x01800000

#define MAX_NAME_SIZE 20
#define MAX_NS_SIZE 50
#define NS_TID 2

#define NS_TYPE_WHOIS 1
#define NS_TYPE_REGISTERAS 2
#define NS_TYPE_REPLY 3

struct nameserver{
  char name[MAX_NAME_SIZE];
  int tid;
};

struct ns_request {
	int type;
	char * name;
  int tid;
};


int RegisterAs( char *name );

int WhoIs( char *name );

void nameserver(void);

#endif /* NAMESERVER_H */
