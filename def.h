#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pvm3.h>
#include <map>
#include <list>
#include "messages.h"

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
#define REQUEST_MIN 	1
#define REQUEST_MAX 	15

#define MSG_MSTR 		1
#define MSG_SLV  		2

/* init process */
#define MSG_INIT 		0x00
#define MSG_STOP 		0x01

/* node requesting or revoking a request */
#define MSG_REQUEST 	0x10
#define MSG_CANCEL 		0x11

/* resource manager response */
#define MSG_GRANT 		0x20
#define MSG_FREE 		0x21
