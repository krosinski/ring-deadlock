/*
 * node.cpp
 *
 *  Created on: 2009-05-16
 *      Author: chriss, tzok
 */

#include "def.h"

using namespace std;


int detectorTid;
bool busy = true;
int timestamp;
int myId; /* nodes ordinal number */
int tids[NODENUM]; /* node task ids */
int route[NODENUM]; /* 1-next hop */
map<int, int> resources;/* owners */
map<int, list<int> > myResources;/* resource queues */

int requestTime[RESOURCE_NUM]; /* resource request timestamp */

/* currently requested resources */
list<int> request;
list<int> resInUse;
int minNeeded;

inline int pos(int base, int offset) {
	return (base + offset + NODENUM) % NODENUM;
}

inline int abs(int a) {
	return a > 0 ? a : -a;
}

inline int max(int a, int b){
	return a>b ? a : b;
}

/*
 * Read message from some other node
 */
void fetchMsg(Msg& msg, int msgtag) {
	int buf[BUF_SIZE];
	msg.setType(msgtag);
	int size = msg.size - 1;

	pvm_upkint(buf, size, 1);
	msg.setMsg(myId, buf);

	if (msgtag & USER_MSG_MASK)
		timestamp = max(msg.timestamp, timestamp) + 1;
}

/*
 * Send message to some other node
 */
void sendMsg(Msg msg) {
	int buf[BUF_SIZE];

	if (msg.type & USER_MSG_MASK)
		msg.timestamp = ++timestamp;


	int size = msg.toIntArray(buf);


	if (msg.dest == myId) {
		printf("%d HELL NO! ;-P \n", myId);
		switch (msg.type) {
		case MSG_REQUEST:
			printf("%d REQUEST BUG!!!\n", myId);
			break;
		case MSG_CANCEL:
			printf("%d CANCEL BUG!!!\n", myId);
			break;

		case MSG_FREE:
			printf("%d FREE BUG!!!\n", myId);
			break;

		case MSG_GRANT:
			printf("%d GRANT BUG!!!\n", myId);
			break;
		default:
			break;

		}
		fflush(0);
		fflush(0);
	}
	pvm_initsend(PvmDataDefault);
	pvm_pkint(buf, size, 1);
	pvm_send(tids[route[msg.dest]], msg.type);
}

/*
 * Forward message
 */
void forwardMsg(int destination, int msgtag) {
	int size = Msg::getSize(msgtag) - 1;
	int dest = destination;


	//printf("*%d forwarding to %d\n", myId, route[dest]);
	/*
	 * Receive remaining data
	 */
	int buf[BUF_SIZE];
	pvm_upkint(buf, size, 1);

	/*
	 * Pack the data
	 */
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&dest, 1, 1);
	pvm_pkint(buf, size, 1);

	/*
	 * Route it
	 */
	pvm_send(tids[route[dest]], msgtag);
}

/*
 * Grant a resource
 */
void grantResource(int resId) {
	if (myResources[resId].empty())
		return;

	if (*myResources[resId].begin()==myId)
		resourceGranted(resId);

	Msg msg;
	msg.setType(MSG_GRANT);
	msg.data = resId;
	msg.from = myId;
	msg.dest = *myResources[resId].begin();
	sendMsg(msg);
}

/*
 * Free a resource
 */
void freeResources() {

	list<int>::iterator it;
	printf("%d (t:%d)Releasing resources: ", myId, timestamp);
	for (it = resInUse.begin(); it != resInUse.end(); ++it)
		printf("%d ", *it);
	printf("\n");
	fflush(0);
	fflush(0);

	Msg msg;
	msg.setType(MSG_FREE);
	msg.from = myId;

	for (it = resInUse.begin(); it != resInUse.end(); ++it) {
		if (resources[*it] == myId) {
			myResources[*it].erase(myResources[*it].begin());
			grantResource(*it);
		} else {
			msg.data = *it;
			msg.dest = resources[*it];
			sendMsg(msg);
		}
	}
	resInUse.clear();
}

/*
 * Cancel resource request
 */
void cancelResources() {

	list<int>::iterator it;
	printf("%d (t:%d)Cancelling resources: ", myId, timestamp);
	for (it = request.begin(); it != request.end(); ++it)
		printf("%d ", *it);
	printf("\n");
	fflush(0);
	fflush(0);

	Msg msg;
	msg.setType(MSG_CANCEL);
	msg.from = myId;

	for (it = request.begin(); it != request.end(); ++it) {
		if (resources[*it] == myId) {
			myResources[*it].remove(myId);
			grantResource(*it);//check if sb is waiting for res
		} else {
			msg.data = *it;
			msg.dest = resources[*it];
			sendMsg(msg);
		}
	}
	request.clear();
}

/*
 * Action when a resource was granted
 */
void resourceGranted(int resId) {
	printf("%d (t:%d)Res GRANTED %d\n", myId,timestamp, resId);
	fflush(0);
	fflush(0);

	if (resInUse.size() < minNeeded) {
		resInUse.insert(resInUse.begin(), resId);
		request.remove(resId);
	}

	if ((resInUse.size() >= minNeeded)&&(!request.empty()))
		cancelResources();
}

/*
 * Request a resource
 */
void requestRes() {
	request.clear();
	resInUse.clear();

	/*
	 * Prepare list of available resources
	 */
	int i;
	list<int> available;
	for (i = 0; i < RESOURCE_NUM; ++i)
		available.insert(available.begin(), i);

	/*
	 * Get random number of resources to request
	 */
	int count = REQUEST_MIN + rand() % (REQUEST_MAX - REQUEST_MIN + 1);
	minNeeded = 1 + rand() % count;

	/*
	 * Generate random request list
	 */
	list<int>::iterator it;
	while (count--) {//get $count resources from all possible
		i = 0;
		it = available.begin();
		int pos = rand() % available.size();
		while (i++ < pos)
			++it;

		request.insert(request.begin(), *it);
		available.erase(it);
	}

	printf("\n%d (t:%d)Resources needed(%d): ", myId, timestamp, minNeeded);
	for (it = request.begin(); it != request.end(); ++it)
		printf("%d ", *it);
	printf("\n");
	fflush(0);
	fflush(0);

	/*
	 * Request resources
	 */
	printf("%d (t:%d)my resources: ", myId, timestamp);
	for (it = request.begin(); it != request.end(); ++it) {
		if (resources[*it] == myId) {//add myself to my queue ^^
			if (myResources[*it].empty()) {//
				resInUse.insert(resInUse.begin(), *it);
				printf("%d ", *it);
			}
			myResources[*it].insert(myResources[*it].end(), myId);

		} else {//send requests
			Msg msg;
			msg.setType(MSG_REQUEST);
			msg.dest = resources[*it];
			msg.from = myId;
			msg.data = *it;
			sendMsg(msg);
			requestTime[*it] = timestamp;
		}
	}

	printf("\n");
	fflush(0);
	fflush(0);
	for (it = resInUse.begin(); it != resInUse.end(); ++it)
		request.remove(*it);
}

/*
 * Prepare routing table
 */
void createRoute() {
	//int edge = pos(myId, NODENUM / 2);
	int next;
	route[myId] = -1;
	for (int i = myId + 1; i < NODENUM + myId; ++i) {
		if (i - myId < NODENUM / 2)
			next = +1;
		else
			next = -1;

		route[i % NODENUM] = pos(myId, next);
	}
}

///////////////////////////////////////////////////////////////////////////////
int main() {
	/*
	 * Wait until all other nodes are active
	 */


	pvm_joingroup("init");
	pvm_barrier("init", NODENUM + 1);

	/*
	 * Read data
	 */
	int i, resOwner;

	pvm_recv(-1, MSG_INIT);
	pvm_upkint(&myId, 1, 1);
	pvm_upkint(tids, NODENUM, 1);
	for (i = 0; i < RESOURCE_NUM; ++i) {
		pvm_upkint(&resOwner, 1, 1);
		resources[i] = resOwner;
	}



	/*
	 * Prepare routing over token topology
	 */
	srand(tids[myId]);
	createRoute();


	timestamp = 0;

	for(i=0;i<RESOURCE_NUM;++i)
		requestTime[i] = 0;

	/*
	 * ACTIVITY
	 */
	while (true) {
		/*
		 * Prepare timeout structure
		 */
		struct timeval to;
		to.tv_sec = 1;
		to.tv_usec = 0;

		/*
		 * If no requests were made and no resources are in use
		 */
		sleep(1 + rand() % 5);

		if ((myId < 666) && request.empty() && resInUse.empty())//my < x -> for debug purpose
			if (((float) rand() / (float) RAND_MAX) < REQUEST_PROB) {
				requestRes();
				if (resInUse.size() < minNeeded)
					busy = false;
				else
					busy = true;
			}

		/*
		 * w
		 */
		if (request.empty() && !resInUse.empty()) {
			busy = true;
			if (((float) rand() / (float) RAND_MAX) < FREE_PROB)
				freeResources();
		}

		/*
		 * Check incoming messages
		 */
		int bufid;
		usleep(100);
		if (!(bufid = pvm_trecv(-1, -1, &to)))
			continue;

		/*
		 * Read incoming message
		 */
		int dest, msgsize, msgtag, sender;
		pvm_bufinfo(bufid, &msgsize, &msgtag, &sender);
		pvm_upkint(&dest, 1, 1);

		/*
		 * Forward message
		 */
		if (dest != myId) {
			//printf("* %d dest %d *\n", myId, dest);
			forwardMsg(dest, msgtag);
			continue;
		}

		/*
		 * Or process it
		 */
		Msg msg;
		if (msgtag & NON_MANAGING)
			fetchMsg(msg, msgtag);

		switch (msgtag) {
		case MSG_REQUEST:
			printf("%d (t:%d)Resource Request by: %d resId: %d\n", myId,timestamp, msg.from,
					msg.data);

			/*
			 * Put the resource into the queue and grant it immediately if it
			 * was free before
			 */
			myResources[msg.data].push_back(msg.from);
			if (*myResources[msg.data].begin() == msg.from) {
				printf("granted!\n");
				grantResource(msg.data);

			} else
				printf("%d putting  %d on queue for res: %d\n", myId, msg.from, msg.data);

			break;
		case MSG_FREE:
			printf("%d (t:%d)Resource released: %d\n", myId,timestamp, msg.data);
			/*
			 * Grant the resource to the next node on the list
			 */
			if (*myResources[msg.data].begin() == msg.from) {
				myResources[msg.data].pop_front();
				grantResource(msg.data);
			}
			break;
		case MSG_CANCEL:
			printf("%d (t:%d)Resource canceled %d\n", myId,timestamp, msg.data);
			/*
			 * Remove the cancelling node from the list and grant the
			 * resource to some other node if necessary
			 */
			if (*myResources[msg.data].begin() == msg.from) {
				myResources[msg.data].pop_front();
				grantResource(msg.data);
			} else
				myResources[msg.data].remove(msg.from);
			break;
		case MSG_GRANT:
			if (msg.timestamp <= requestTime[msg.data]){//the message is late - epic fail =(
				printf("%d (t:%d)Grant ignored(late message) res: %d\n", myId,timestamp, msg.data);
				break;
			}
			resourceGranted(msg.data);
			break;

		case MSG_FLOOD:
			//TODO flood message processing
			break;

		case MSG_ECHO:
			//TODO echo message processing
			break;
		case MSG_SHORT:
			//TODO short message processing
			break;

		case MSG_START_ALG:
			pvm_upkint(&detectorTid,1,1);
			//TODO start failure detection [node 0 only]
			break;
		default:
			break;
		}

		fflush(0);
		fflush(0);

	}
	pvm_exit();
}
