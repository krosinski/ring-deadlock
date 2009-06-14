/*
 * messages.h
 *
 *  Created on: 2009-05-28
 *      Author: chriss
 */



#ifndef MESSAGES_H_
#define MESSAGES_H_


#define PRECISION 1000000

class Msg {

public:
	static const int BASE_SIZE = 2;
	int type;//msg type
	int from;//msg source
	int dest;//msg destination
	int timestamp;//lamport clock
	int data;//data

	float weight;//  for deadlock detection ;-P
	int starttime;// deadlock detection start time
	int init;// deadlock detection


	int size;//

	Msg();
	Msg(int from, int dest, int type);
	~Msg();
	static int getSize(int msgtype);
	int toIntArray(int* buf);
	int setMsg(int dest, int* buf);
	void setType(int type);
};

#endif /* MESSAGES_H_ */
