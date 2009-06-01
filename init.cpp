/*
 * init.cpp
 *
 *  Created on: 2009-05-16
 *      Author: chriss, tzok
 */

#include <time.h>
#include "def.h"

using namespace std;

/*
 * Initializes an assigment of resources to its controllers
 */
void initResources(map<int, int>& res) {
	int i = 0;
	/*
	 * Provide at least 1 resource per node
	 */
	for (i = 0; i < NODENUM; ++i)
		res[i] = i;

	/*
	 * Assign the rest
	 */
	for (i = NODENUM; i < RESOURCE_NUM; ++i)
		res[i] = rand() % NODENUM;
}

///////////////////////////////////////////////////////////////////////////////
int main() {
	/* resource id -> owner */
	map<int, int> resources;
	map<int, int>::iterator it;

	/*
	 * Initialize resource assignment
	 */
	initResources(resources);

	/*
	 * Spawn all the children processes and wait until all are active
	 */
	printf("Spawning nodes\n");
	fflush(0);
	int tids[NODENUM];
	int childrenCount = pvm_spawn(NODENAME, NULL, PvmTaskDefault, "", NODENUM,
			tids);
	pvm_joingroup("init");
	pvm_barrier("init", NODENUM + 1);

	/*
	 * Send all nodes needed information
	 */
	int i, j;
	for (i = 0; i < childrenCount; i++) {
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&i, 1, 1);
		pvm_pkint(tids, NODENUM, 1);

		for (j = 0; j < RESOURCE_NUM; ++j)
			pvm_pkint(&resources[j], 1, 1);

		printf("Wysylam %d\n", i);
		pvm_send(tids[i], MSG_INIT);
	}

	ofstream outputFile;
	outputFile.open("/tmp/node0.txt");
	outputFile << tids[0];
	outputFile.close();



	pvm_exit();
}
