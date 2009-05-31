/*
 * init.cpp
 *
 *  Created on: 2009-05-16
 *      Author: chriss
 */

#include "def.h"

using namespace std;

void initResources(map<int,int>& res){
	int i = 0;
	/* provide at least 1 resource per node*/
	for(i=0;i<NODENUM;++i)
		res[i] = i;


	for(i=NODENUM;i<RESOURCE_NUM;++i)
		res[i] = rand()%NODENUM;
}


int main(){

	int mytid;
	int tids[NODENUM];		/* slave task ids */
	char slave_name[NAMESIZE];
	int nproc, i,j, who;

	srand(0);

	/* resource id -> owner */
	map<int, int> resources;
	map<int,int>::iterator it;

	initResources(resources);

	mytid = pvm_mytid();







//	return 1;
	printf("Spawning nodes\n");
	fflush(0);

	nproc=pvm_spawn(NODENAME, NULL, PvmTaskDefault, "", NODENUM, tids);

	pvm_joingroup("init");

	sleep(1);
	pvm_barrier("init", NODENUM +1);


	for( i=0 ; i<nproc ; i++ )
	{
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&i,1,1);
		pvm_pkint(tids, NODENUM, 1 );


		for(j=0;j< RESOURCE_NUM;++j)
			pvm_pkint(&resources[j],1,1);


		printf("Wysylam %d\n", i);
		pvm_send(tids[i], MSG_INIT);


	}





	pvm_exit();
}

