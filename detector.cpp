/*
 * node.cpp
 *
 *  Created on: 2009-05-16
 *      Author: chriss, tzok
 */

#include "def.h"

using namespace std;

int main(int argc, char** argv) {
	printf("Deadlock detection started\n");
	fflush(0);

	string temp[20];
	ifstream ifs(TMP_FILE);
	int i=0;
	while (!ifs.eof()){
		getline(ifs, temp[i++]);
		printf(temp[i-1].c_str());
		printf("\n");
	}
	int num = atoi(argv[1]);
	int nodeToCheck = atoi(temp[num].c_str());

	printf("nodeToCheck = %d tid: %X\n",num, nodeToCheck);
	fflush(0);

	/*
	 * FIXME
	 * Send signal to start deadlock detection
	 * to selected node
	 */
	int dest = 0;
	int mytid = pvm_mytid();
	int timestamp = 0;
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&dest, 1, 1);
	pvm_pkint(&mytid, 1, 1);
	pvm_pkint(&timestamp, 1, 1);
	pvm_send(nodeToCheck, MSG_START_ALG);

	/*
	 * FIXME
	 * Receive answer from given node
	 */
	int isDeadlock;
	pvm_recv(nodeToCheck, MSG_DEADLOCK_INFO);
	pvm_upkint(&isDeadlock, 1, 1);

	if (isDeadlock == NO_DEADLOCK)
		printf("Deadlock was NOT DETECTED on node %X\n", nodeToCheck);
	else
		printf("Deadlock DETECTED on node %X\n", nodeToCheck);
	fflush(0);

	pvm_exit();
}
