#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <pvm3.h>

#include <map>
#include <list>
#include <iostream>
#include <fstream>
#include <set>
#include <string>

#include "messages.h"

#define TMP_FILE		"/tmp/node0.txt"
#define NODENAME		"node"
#define NODENUM			10

#define NAMESIZE   		64
#define BUF_SIZE 		32

/* number of resources in the system - at least 1 for each node */
#define RESOURCE_NUM 	30

/* probability of requesting and freeing resources */
#define REQUEST_PROB 	0.75
#define FREE_PROB 		0.5

/* number of requested resources */
#define REQUEST_MIN 	2
#define REQUEST_MAX 	8

#define MSG_MSTR 		1
#define MSG_SLV  		2

/* masks */
#define USER_MSG_MASK	0x10
#define NON_MANAGING	0x30

/* init process */
#define MSG_INIT 		0x00
#define MSG_STOP 		0x01
#define MSG_START_ALG	0x02

/* node requesting or revoking a request */
#define MSG_REQUEST 	0x10
#define MSG_CANCEL 		0x11
#define MSG_FREE 		0x12

/* resource manager response */
#define MSG_GRANT 		0x13

/* deadlock detection messages */
#define MSG_FLOOD			0x20
#define MSG_ECHO			0x21
#define MSG_SHORT			0x22
#define MSG_DEADLOCK_INFO	0x23

#define	NO_DEADLOCK			0
#define DEADLOCK			1

#define EPS					0.001

/* structures */
typedef struct {
	bool flag; /* set to true after first snapshot is done */
	std::list<int> out; /* nodes on which i-th is waiting */
	std::list<int> in; /* nodes waiting on i in the snapshot */
	int t; /* time when init initiated a snapshot */
	bool s; /* local blocked state as seen by snapshot */
	int p; /* value of p as seen by snapshot */
} SNAPSHOT;

typedef struct {
	int from, resource;
} REQUEST;

typedef struct {
	bool wait;
	int time;
	int timeBlock;
	std::list<REQUEST> in; /* list of requests before current */
	std::list<int> out; /* list of resources wanted by current node */
	std::list<int> inUse; /* list of resources granted */
	std::map<int, int> granted; /* list of resources granted [which] = who*/
	bool resFree[RESOURCE_NUM]; /* array of booleans if res is free or not */
	int p;
	float weight;
	int init;
	int starttime;
} STATE;
