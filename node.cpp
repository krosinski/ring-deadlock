/*
 * node.cpp
 *
 *  Created on: 2009-05-16
 *      Author: chriss
 */

#include "def.h"

using namespace std;

bool busy = true;
int myId; /* nodes ordinal number */
int tids[NODENUM]; /* node task ids */
int route[NODENUM]; /* 1-next hop */
map<int, int> resources;/* owners */
map<int, list<int> > myResources;/* who currently uses them / -1 available */

/* currently requested resources */
list<int> request;
list<int> resInUse;
int minNeeded;





inline int pos(int base, int offset) {
	return (base + offset + NODENUM) % NODENUM;
}

void fetchMsg(Msg& msg, int msgtag) {
	int buf[BUF_SIZE];
	msg.setType(msgtag);
	int size = msg.size - 1;

	pvm_upkint(buf, size, 1);
	msg.setMsg(myId, buf);
}

void sendMsg(Msg msg) {

	int buf[BUF_SIZE];

	int size = msg.toIntArray(buf);

	pvm_initsend(PvmDataDefault);
	pvm_pkint(buf, size, 1);
	pvm_send(tids[route[msg.dest]], msg.type);

}

void forwardMsg(int destination, int msgtag) {



	int size = Msg::getSize(msgtag) - 1;

	int buf[BUF_SIZE];
	int dest = destination;
	/* rcv remaining data */

	pvm_upkint(buf, size, 1);

	/* pack the data */
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&dest, 1, 1);
	pvm_pkint(buf, size, 1);

	/* route it */
	pvm_send(tids[route[dest]], msgtag);

}


void grantResource(int resId){


	if (myResources[resId].empty())
		return;

	Msg msg;
	msg.setType(MSG_GRANT);
	msg.data = resId;
	msg.from = myId;
	msg.dest = *myResources[resId].begin();

	sendMsg(msg);
}


void freeResources(){


	list<int>::iterator it;

	Msg msg;
	msg.setType(MSG_FREE);
	msg.from = myId;

	for(it=resInUse.begin();it!=resInUse.end();++it){
		if (resources[*it]==myId){
			myResources[*it].erase(myResources[*it].begin());
			grantResource(*it);
		}
		else{
			msg.dest = resources[*it];
			msg.data = *it;
			sendMsg(msg);
		}
	}

	resInUse.clear();

}

void cancelResources(){


	list<int>::iterator it;
	Msg msg;
	msg.setType(MSG_CANCEL);
	msg.from = myId;
	for(it = request.begin();it!=request.end();++it){
		if (resources[*it]==myId){
			myResources[*it].erase(myResources[*it].begin());
			grantResource(*it);//check if sb is waiting for res
		}
		else{
			msg.data = *it;
			msg.dest = resources[*it];
			sendMsg(msg);
		}
	}

	request.clear();
}



void resourceGranted(int resId){

;
	resInUse.insert(resInUse.begin(), resId);
	request.remove(resId);

	if(resInUse.size()>= minNeeded)//cancel remaining requests
		cancelResources();

}

void requestRes(){
	list<int> available;

	list<int>::iterator it;

	int i,pos;
	Msg msg;

	request.clear();
	resInUse.clear();

	for(i=0;i< RESOURCE_NUM ;++i)
		available.insert(available.begin(),i);

	int count = REQUEST_MIN + rand()%(REQUEST_MAX - REQUEST_MIN + 1);


	minNeeded = 1 + rand()%count;

	while(count--){//get $count resources from all possible
		it = available.begin();
		pos = rand()%available.size();
		while(i++ < pos)
			++it;

		request.insert(request.begin(), *it);
		available.erase(it);
	}

	for(it = request.begin();it!=request.end();++it){
		if (resources[*it]==myId){//add myself to my queue ^^
			if(myResources[*it].empty()){//
				resInUse.insert(resInUse.begin(),*it);
			}
			myResources[*it].insert(myResources[*it].end(), myId);
		}
		else{//send requests
			//continue;

			msg.setType(MSG_REQUEST);
			msg.dest = resources[*it];
			msg.from = myId;
			msg.data = *it;
			sendMsg(msg);
		}



	}
	for(it=resInUse.begin();it!=resInUse.end();++it)
		request.remove(*it);


	//printf("%d: req: %d got: %d needed: %d\n", myId, request.size(), resInUse.size(), minNeeded );

}


void createRoute() {
	int edge = pos(myId, NODENUM / 2);
	route[myId] = -1;
	for (int i = 0; i < NODENUM; ++i) {
		if (i == myId)
			continue;
		if (edge > myId)
			if ((i > myId) && (i <= edge))
				route[i] = pos(myId, +1); //+ unarny ;-P
			else
				route[i] = pos(myId, -1);
		else if ((i < myId) && (i >= edge))
			route[i] = pos(myId, +1);
		else
			route[i] = pos(myId, -1);
	}
}

int main() {

	struct timeval to;
	int sender;
	int dest;
	int msgtag;
	int msgsize;
	int bufid;
	int resOwner;

	int mytid;
	char slave_name[NAMESIZE];
	int nproc, i, who;


	/**** INIT NODE ****/
	srand(time(0));
	pvm_joingroup("init");
	pvm_barrier("init", NODENUM + 1);

	pvm_recv(-1, MSG_INIT);
	pvm_upkint(&myId, 1, 1);
	pvm_upkint(tids, NODENUM, 1);

	for (i = 0; i < RESOURCE_NUM; ++i) {
		pvm_upkint(&resOwner, 1,1 );
		resources[i] = resOwner;
	}

	createRoute();




	fflush(0);
	fflush(0);

	Msg msg;
	//return 0;

	while (true) {
		to.tv_sec = 1;
		to.tv_usec = 0;

		//printf("%d busy %d\n", myId, busy);
		//fflush(0);
		//fflush(0);


		if(request.empty() && resInUse.empty())
			if (((float)rand()/RAND_MAX)< REQUEST_PROB){
				requestRes();
				if (resInUse.size()< minNeeded)
					busy = false;
				else
					busy = true;
			}


		if(request.empty() && !resInUse.empty()){
			busy = true;

			if(((float)rand()/RAND_MAX)< FREE_PROB)
				freeResources();
		}



		usleep(100);
		if (!(bufid = pvm_trecv(-1, -1, &to)))
			continue;//no message


		pvm_bufinfo(bufid, &msgsize, &msgtag, &sender);

		pvm_upkint(&dest, 1, 1);
		/* forward message */
		if (dest != myId) {
			forwardMsg(dest, msgtag);
			//printf("%d forward, od %d do %d \n", myId, -1, route[dest]);
			continue;
		} else {
			//printf("%d Dostalem wiadomosc \n", myId);
			fetchMsg(msg, MSG_REQUEST);
			//printf("val: %d \n", msg.data);
		}
		/* else process it */
		switch (msgtag)//process msg
		{
		case MSG_REQUEST:
			myResources[msg.data].push_back(msg.from);//put the request to the queue
			if(*myResources[msg.data].begin() == msg.from)//if the resource was free
				grantResource(msg.data);
			break;
		case MSG_FREE:
			if(*myResources[msg.data].begin() == msg.from){
				myResources[msg.data].pop_front();
				grantResource(msg.data);//if any1 is waiting
			}
			break;
		case MSG_CANCEL:
			if(*myResources[msg.data].begin()== msg.from){//if the res was assigned before cancel req.
				myResources[msg.data].pop_front();
				grantResource(msg.data);
			}
			else//remove from the queue
				myResources[msg.data].remove(msg.from);

			break;
		case MSG_GRANT:
			resourceGranted(msg.data);
			break;


		default:
			break;
		}

		break;

	}

	pvm_exit();
}

