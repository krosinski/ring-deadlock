/*
 * node.cpp
 *
 *  Created on: 2009-05-16
 *      Author: chriss, tzok
 */

#include "def.h"

using namespace std;

int main() {

	int mytid;
	int node_tid;

	int nproc, i, who;

	mytid = pvm_mytid();

	//gethostname(slave_name,NAMESIZE);


	printf("detector started\n");
	fflush(0);
	fflush(0);

	int dest = 0;
	ifstream ifs("/tmp/node0.txt");
	string temp;

	getline(ifs, temp);
	node_tid = atoi(temp.c_str());

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&dest, 1, 1);
	pvm_pkint(&mytid,1, 1);
	pvm_send(node_tid, MSG_START_ALG);

	//todo recieve status

	pvm_exit();

}

