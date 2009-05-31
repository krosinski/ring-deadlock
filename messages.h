/*
 * messages.h
 *
 *  Created on: 2009-05-28
 *      Author: chriss
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

class Msg{

public:
	static const int BASE_SIZE = 2;
	int type;//msg type
	int from;//msg source
	int dest;//msg destination
	int data;
	int size;


	Msg();
	Msg(int from, int dest, int type);
	~Msg();
	static int getSize(int msgtype);
	int toIntArray(int* buf);
	int setMsg(int dest, int* buf);
	int setType(int type);
};

class RequestMsg : public Msg{

public:
	static const int SIZE = 3;
	int data;

	RequestMsg();
	RequestMsg(int from, int dest, int type, int data);
	~RequestMsg();
	int toIntArray(int* buf);
	int setMsg(int dest, int* buf);
};



#endif /* MESSAGES_H_ */
