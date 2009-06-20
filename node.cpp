/*
 * node.cpp
 *
 *  Created on: 2009-05-16
 *      Author: chriss, tzok
 */

#include "def.h"
using namespace std;

inline int abs(int);
inline void cancel(int, int, int);
void createRoute(void);
void fetchMessage(Msg&, int);
void forwardMessage(int, int);
void free_(int, int, int);
void listRemove(list<REQUEST>&, REQUEST&);
inline int max(int, int);
void message(int, int, int, int);
inline int pos(int, int);
void randomizeRequests(void);
void receiveCancel(REQUEST&);
void receiveFree(REQUEST&);
void receiveReply(int);
void receiveRequest(REQUEST&);
inline int repliesNeeded(void);
inline void reply(int, int, int);
inline void request(int, int, int);
void sendMessage(Msg&);
void sendReply(REQUEST&);
void sendRequest();

int detectorId, myId;
int requestTime[RESOURCE_NUM], resources[RESOURCE_NUM], route[NODENUM],
		tids[NODENUM];

STATE state;
SNAPSHOT snapshot;

list<int> requests;

/*
 * abs()
 */
inline int abs(int a) {
	return a > 0 ? a : -a;
}

/*
 * cancel()
 */
inline void cancel(int from, int to, int res) {
	if (from != to) {
		message(from, to, res, MSG_CANCEL);
		return;
	}
	REQUEST req;
	req.from = from;
	req.resource = res;
	receiveCancel(req);
}

/*
 * createRoute()
 * Prepare routing table
 */
void createRoute() {
	route[myId] = -1;
	int next;
	for (int i = myId + 1; i < NODENUM + myId; ++i) {
		if (i - myId < NODENUM / 2)
			next = +1;
		else
			next = -1;
		route[i % NODENUM] = pos(myId, next);
	}
}

/*
 * fetchMessage()
 * Read message from some other node
 */
void fetchMessage(Msg& msg, int msgtag) {
	int buf[BUF_SIZE];
	msg.setType(msgtag);
	int size = msg.size - 1;

	pvm_upkint(buf, size, 1);
	msg.setMsg(myId, buf);

	if (msgtag & USER_MSG_MASK)
		state.time = max(msg.timestamp, state.time) + 1;
}

/*
 * forwardMsg()
 * Forward message
 */
void forwardMessage(int destination, int msgtag) {
	int size = Msg::getSize(msgtag) - 1;
	int dest = destination;

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
 * free_()
 */
void free_(int from, int to, int res) {
	if (from != to) {
		message(from, to, res, MSG_FREE);
		return;
	}
	REQUEST req;
	req.from = from;
	req.resource = res;
	receiveFree(req);
}

/*
 * listRemove()
 * Removes one REQUEST from the list
 */
void listRemove(list<REQUEST> &l, REQUEST &req) {
	list<REQUEST>::iterator i;
	for (i = l.begin(); i != l.end(); ++i)
		if ((i->from == req.from) && (i->resource == req.resource)) {
			l.erase(i);
			break;
		}
}

/*
 * max()
 */
inline int max(int a, int b) {
	return a > b ? a : b;
}

/*
 * message()
 */
void message(int from, int to, int res, int type) {
	Msg msg;
	msg.data = res;
	msg.dest = to;
	msg.from = from;
	msg.size = msg.getSize(type);
	msg.timestamp = state.time++;
	msg.type = type;
	sendMessage(msg);
}

/*
 * pos()
 */
inline int pos(int base, int offset) {
	return (base + offset + NODENUM) % NODENUM;
}

/*
 * randomizeRequests()
 */
void randomizeRequests() {
	/*
	 * Shuffle resource allocation
	 */
	int resources[RESOURCE_NUM];
	int i, j;
	for (i = 0; i < RESOURCE_NUM; ++i)
		resources[i] = i;
	for (i = 1; i < RESOURCE_NUM; ++i) {
		j = rand() % i;
		int temp = resources[i];
		resources[i] = resources[j];
		resources[j] = temp;
	}

	/*
	 * Get random number of resources needed
	 */
	int requestCount = REQUEST_MIN + rand() % (REQUEST_MAX - REQUEST_MIN + 1);
	for (i = 0; i < requestCount; ++i)
		requests.push_back(resources[i]);
}

/*
 * receiveCancel()
 * Executed by node i when it receives a cancel from j
 */
void receiveCancel(REQUEST &req) {
	map<int, int>::iterator i = state.granted.find(req.resource);
	if ((i != state.granted.end()) && (state.granted[req.resource] == req.from)) {
		/* FIXME */
		printf("FREE instead of CANCEL\n");
		fflush(0);
		REQUEST temp;
		temp.from = req.from;
		temp.resource = req.resource;
		receiveFree(temp);
		printf("FREE received successfully\n");
		fflush(0);
		return;
	}
	else
		listRemove(state.in, req);
}

/*
 * FIXME
 * receiveFree()
 */
void receiveFree(REQUEST &req) {
	listRemove(state.in, req);
	printf("listRemove() CORRECT\n");
	fflush(0);

	state.granted.erase(req.resource);
	printf("erase CORRECT\n");
	fflush(0);

	list<REQUEST>::iterator i;
	for (i = state.in.begin(); i != state.in.end(); ++i)
		if (i->resource == req.resource) {
			/* FIXME Tutaj chyba leży błąd */
			state.granted[req.resource] = i->from;
			req.from = i->from;
			sendReply(req);
			printf("loop in CORRECT\n");
			fflush(0);
			break;
		}
	printf("loop out CORRECT\n");
	fflush(0);
}

/*
 * receiveReply()
 * Executed by node i when it receives a reply from j to its request
 */
void receiveReply(int resource) {
	state.inUse.push_back(resource);

	list<int>::iterator i, j;
	for (i = state.out.begin(); i != state.out.end(); ++i) {
		if (*i == resource) {
			state.out.erase(i);
			state.p--;
			if (!state.p) {
				state.wait = false;
				for (j = state.out.begin(); j != state.out.end(); ++j)
					cancel(myId, resources[*j], *j);
				state.out.clear();
				for (j = state.inUse.begin(); j != state.inUse.end(); ++j)
					free_(myId, resources[*j], *j);
				state.inUse.clear();
			}
			break;
		}
	}
}

/*
 * receiveRequest()
 * Executed by node i when it receives a request made by j
 */
void receiveRequest(REQUEST &request) {
	state.in.push_back(request);
}

/*
 * repliesNeeded()
 */
inline int repliesNeeded() {
	return 1; //state.out.size() / 2;
}

/*
 * reply()
 */
void reply(int from, int to, int res) {
	if (from != to) {
		message(from, to, res, MSG_GRANT);
		return;
	}
	receiveReply(res);
}

/*
 * request()
 */
void request(int from, int to, int res) {
	if (from != to) {
		message(from, to, res, MSG_REQUEST);
		return;
	}
	REQUEST req;
	req.from = from;
	req.resource = res;
	receiveRequest(req);
}

/*
 * sendMessage()
 * Send message to some other node
 */
void sendMessage(Msg &msg) {
	int buf[BUF_SIZE];

	if (msg.type & USER_MSG_MASK)
		msg.timestamp = ++state.time;

	int size = msg.toIntArray(buf);
	pvm_initsend(PvmDataDefault);
	pvm_pkint(buf, size, 1);
	pvm_send(tids[route[msg.dest]], msg.type);
}

/*
 * sendReply()
 * Executed by node i when it replies to a request by j
 */
void sendReply(REQUEST &req) {
	listRemove(state.in, req);
	state.granted[req.resource] = req.from;
	reply(myId, req.from, req.resource);
}

/*
 * sendRequest()
 * Executed by node i when it blocks on a p-out-of-q request
 */
void sendRequest() {
	state.out.clear();

	list<int>::iterator i;
	for (i = requests.begin(); i != requests.end(); ++i) {
		state.out.push_back(*i);
		request(myId, resources[*i], *i);
		requestTime[*i] = state.time;
	}
	state.p = repliesNeeded();
	state.timeBlock = state.time;
	state.wait = true;
}

///////////////////////////////////////////////////////////////////////////////
int main() {
	/*
	 * Local state and snapshot
	 */
	state.wait = false;
	state.time = 0;
	state.timeBlock = 0;
	state.in.clear();
	state.out.clear();
	state.inUse.clear();
	state.granted.clear();
	state.p = 0;
	state.weight = 1.0;

	snapshot.out.clear();
	snapshot.in.clear();
	snapshot.t = -1;
	snapshot.s = false;

	/*
	 * Wait until all other nodes are active
	 */
	pvm_joingroup((char*) "init");
	pvm_barrier((char*) "init", NODENUM + 1);

	/*
	 * Read data
	 */
	pvm_recv(-1, MSG_INIT);
	pvm_upkint(&myId, 1, 1);
	pvm_upkint(tids, NODENUM, 1);
	pvm_upkint(resources, RESOURCE_NUM, 1);
	srand(tids[myId] * time(0));

	/*
	 * Prepare routing over token topology
	 */
	createRoute();
	memset(requestTime, 0, sizeof(requestTime));

	/*
	 * ACTIVITY
	 */
	bool running = true;
	int initId;
	while (running) {
		/*
		 * Random "processing" time
		 */
		//sleep(rand() & 3);

		/*
		 * Randomize requests
		 */
		if (!state.wait) {
			randomizeRequests();
			sendRequest();
		}

		/*
		 * Check incoming messages
		 */
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		int bufid = pvm_trecv(-1, -1, &timeout);
		if (!bufid)
			continue;

		/*
		 * Read incoming message
		 */
		int from, to, size, tag;
		pvm_bufinfo(bufid, &size, &tag, &from);
		pvm_upkint(&to, 1, 1);

		/*
		 * Forward message
		 */
		if (to != myId) {
			forwardMessage(to, tag);
			continue;
		}

		/*
		 * Or process it
		 */
		Msg msg;
		if (tag & NON_MANAGING)
			fetchMessage(msg, tag);

		/* FIXME */

		char *action;
		switch (msg.type) {
		case MSG_REQUEST:
			action = "REQUEST";
			break;
		case MSG_CANCEL:
			action = "CANCEL";
			break;
		case MSG_FREE:
			action = "FREE";
			break;
		case MSG_GRANT:
			action = "GRANT";
			break;
		case MSG_FLOOD:
			action = "FLOOD";
			break;
		case MSG_SHORT:
			action = "SHORT";
			break;
		case MSG_ECHO:
			action = "ECHO";
			break;
		}
		printf("%d -> %d\t%s\t%d\n", msg.from, msg.dest, action, msg.data);
		fflush(0);

		int i, temp;
		REQUEST req;
		req.from = msg.from;
		req.resource = msg.data;
		switch (tag) {
		case MSG_REQUEST:
			/*
			 * Save the request on a list. Grant immediately if needed
			 */
			receiveRequest(req);
			if (state.granted.find(req.resource) == state.granted.end())
				sendReply(req);
			break;

		case MSG_FREE:
			/*
			 * Grant the resource to the next node on the list
			 */
			receiveFree(req);
			break;

		case MSG_CANCEL:
			receiveCancel(req);
			/* FIXME */
			printf("CANCEL received successfully\n");
			fflush(0);
			break;

		case MSG_GRANT:
			/* TODO Tutaj należy dodać sprawdzanie numeru zgłoszenia */
			if (msg.timestamp > requestTime[msg.data])
				receiveReply(msg.data);
			break;

			/* FIXME FIXME FIXME Tutaj zaczyna się obsługa wiadomości algorytmu detekcji FIXME FIXME FIXME */
		case MSG_FLOOD:
			if (myId != msg.init) {
				if (snapshot.t < msg.starttime) {
					/*
					 * Create new snapshot
					 */
					printf("Valid FLOOD for a new snapshot. ");
					snapshot.t = msg.starttime;
					snapshot.s = state.wait;
					snapshot.in.clear();
					snapshot.in.push_back(msg.from);
					snapshot.out.clear();
					for (list<int>::iterator it = requests.begin(); it
							!= requests.end(); ++it)
						snapshot.out.push_back(*it);

					if (snapshot.s) {
						/*
						 * Node is blocked
						 */
						snapshot.p = state.p;
						printf("Node is blocked. p = %d. Sending FLOOD to: ",
								snapshot.p);
						for (list<int>::iterator it = requests.begin(); it
								!= requests.end(); ++it) {
							printf("%x ", tids[resources[*it]]);
							if (myId == resources[*it]) {
								Msg shortMsg(myId, msg.init, MSG_SHORT);
								shortMsg.weight = msg.weight
										/ (float) requests.size();
								shortMsg.init = msg.init;
								shortMsg.starttime = msg.starttime;
								sendMessage(shortMsg);
							} else {
								Msg flood(myId, resources[*it], MSG_FLOOD);
								flood.weight = msg.weight
										/ (float) requests.size();
								flood.init = msg.init;
								flood.starttime = msg.starttime;
								sendMessage(flood);
							}
						}
						printf("\n");
						fflush(0);
					} else {
						/*
						 * Node is active
						 */
						printf("Node is active. Answering with ECHO to %x\n",
								tids[msg.from]);
						fflush(0);
						snapshot.p = 0;
						Msg echo(myId, msg.from, MSG_ECHO);
						echo.weight = msg.weight;
						echo.init = msg.init;
						echo.starttime = msg.starttime;
						sendMessage(echo);

						snapshot.in.remove(msg.from);
					}
				} else {
					/*
					 * Valid FLOOD for current snapshot
					 */
					printf("Valid FLOOD for current snapshot. ");
					if (!snapshot.s) {
						printf("Answering with ECHO to %x\n", tids[msg.from]);
						Msg echo(myId, msg.from, MSG_ECHO);
						echo.weight = msg.weight;
						echo.init = msg.init;
						echo.starttime = msg.starttime;
						sendMessage(echo);
					} else {
						printf("Answering with SHORT to %x\n", tids[msg.init]);
						Msg shortMsg(myId, msg.init, MSG_SHORT);
						shortMsg.weight = msg.weight;
						shortMsg.init = msg.init;
						shortMsg.starttime = msg.starttime;
						sendMessage(shortMsg);
					}
					fflush(0);
				}
				break;
			}
			/*
			 * There is no "break" for purpose here!!!
			 * If the FLOOD is directed into the initiatior
			 * then pretend it's a SHORT
			 */
		case MSG_SHORT:
			MSG_SHORT_: if (!state.wait) {
				int decision = NO_DEADLOCK;
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&decision, 1, 1);
				pvm_send(detectorId, MSG_DEADLOCK_INFO);
			}
			if ((snapshot.t == msg.starttime) && (snapshot.s == true)) {
				state.weight += msg.weight;
				printf("SHORT for currently initiated snapshot. Weight: %f\n",
						state.weight);
				fflush(0);
				if (state.weight >= 1.0f - EPS) {
					int decision = DEADLOCK;
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&decision, 1, 1);
					pvm_send(detectorId, MSG_DEADLOCK_INFO);
				}
			}
			break;

		case MSG_ECHO:
			if (snapshot.t == msg.starttime) {
				/*
				 * Valid ECHO for current snapshot
				 */
				snapshot.out.remove(msg.from);
				if (!snapshot.s) {
					if (myId == msg.init)
						goto MSG_SHORT_;
					printf("Valid ECHO for current snapshot. ");
					printf("Node is active. Answering with SHORT to %x\n",
							tids[msg.init]);
					fflush(0);
					Msg shortMsg(myId, msg.init, MSG_SHORT);
					shortMsg.weight = msg.weight;
					shortMsg.init = msg.init;
					shortMsg.starttime = msg.starttime;
					sendMessage(shortMsg);
				} else {
					printf("Valid ECHO for current snapshot. ");
					snapshot.p--;
					if (!snapshot.p) {
						snapshot.s = false;
						if (myId == msg.init) {
							printf("Algorithm finished. NO DEADLOCK\n");
							fflush(0);
							int decision = NO_DEADLOCK;
							pvm_initsend(PvmDataDefault);
							pvm_pkint(&decision, 1, 1);
							pvm_send(detectorId, MSG_DEADLOCK_INFO);
							break;
						}
						printf(
								"My need for resources is fulfilled. Sending ECHO to: ");
						for (list<int>::iterator it = snapshot.in.begin(); it
								!= snapshot.in.end(); ++it) {
							printf("%x ", tids[*it]);
							Msg echo(myId, *it, MSG_ECHO);
							echo.weight = msg.weight
									/ (float) snapshot.in.size();
							echo.init = msg.init;
							echo.starttime = msg.starttime;
							sendMessage(echo);
						}
						printf("\n");
						fflush(0);
					} else {
						printf(
								"Node is still blocked. Answering with SHORT to %x\n",
								tids[msg.init]);
						fflush(0);
						if (myId == msg.init)
							goto MSG_SHORT_;
						Msg shortMsg(myId, msg.init, MSG_SHORT);
						shortMsg.weight = msg.weight;
						shortMsg.init = msg.init;
						shortMsg.starttime = msg.starttime;
						sendMessage(shortMsg);
					}
				}
			}
			break;

		case MSG_START_ALG:
			pvm_upkint(&detectorId, 1, 1);

			/*
			 * If selected node is active, it does not look for
			 * deadlock in its neighbourhood
			 */
			if (!state.wait) {
				printf("I am active. No deadlock\n");
				fflush(0);
				int decision = NO_DEADLOCK;
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&decision, 1, 1);
				pvm_send(detectorId, MSG_DEADLOCK_INFO);
				break;
			}

			/*
			 * Create local snapshot
			 */
			printf("Node is blocked. Creating local snapshot\n");
			fflush(0);
			state.weight = 0;
			snapshot.t = state.time;
			snapshot.s = true;
			snapshot.p = state.p;
			snapshot.out.clear();
			for (list<int>::iterator it = requests.begin(); it
					!= requests.end(); ++it)
				snapshot.out.push_back(*it);

			/*
			 * Send FLOOD to all out of q nodes
			 */
			printf("p = %d. Sending FLOOD to all out of q nodes: ", snapshot.p);
			for (list<int>::iterator it = requests.begin(); it
					!= requests.end(); ++it) {
				printf("%x ", tids[resources[*it]]);
				if (myId == resources[*it]) {
					state.weight += 1.0f / (float) requests.size();
					continue;
				}
				Msg flood(myId, resources[*it], MSG_FLOOD);
				flood.weight = 1.0f / (float) requests.size();
				flood.init = myId;
				flood.starttime = state.time;
				sendMessage(flood);
			}
			printf("\n");
			fflush(0);
			break;

		case MSG_STOP:
			pvm_upkint(&initId, 1, 1);
			printf("NODE %d TERMINATED\n", myId);
			fflush(0);
			temp = 0;
			if (initId) {
				for (i = 0; i < NODENUM; ++i) {
					if (i == myId)
						continue;
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&i, 1, 1);
					pvm_pkint(&temp, 1, 1);
					pvm_send(tids[i], MSG_STOP);
				}
			}
			running = false;
			break;

		default:
			break;
		}
	}

	pvm_joingroup((char*) "stop");
	pvm_barrier((char*) "stop", NODENUM);

	if (initId) {
		pvm_initsend(PvmDataDefault);
		pvm_send(initId, MSG_STOP);
		sleep(1);
	}

	pvm_exit();
}
