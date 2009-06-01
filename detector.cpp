/*
 * node.cpp
 *
 *  Created on: 2009-05-16
 *      Author: chriss, tzok
 */

#include "def.h"

int main() {

	int mytid;
	int tids[NODENUM]; /* slave task ids */
	char slave_name[NAMESIZE];
	int nproc, i, who;

	mytid = pvm_mytid();

	//gethostname(slave_name,NAMESIZE);


	pvm_recv(-1, MSG_MSTR);
	pvm_upkint(&who, 1, 1);
	pvm_upkint(&i, 1, 1);
	printf("master: %d: i: %d\n", who, i);

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&mytid, 1, 1);
	pvm_pkstr(slave_name);
	pvm_send(who, MSG_SLV);

	pvm_exit();
}

