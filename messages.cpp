/*
 * messages.cpp
 *
 *  Created on: 2009-05-28
 *      Author: chriss
 */
#include "def.h"

#include <stdio.h>




Msg::Msg(){

}

Msg::Msg(int from, int dest, int type){
	this->dest = dest;
	this->from = from;
	this->setType(type);
}
Msg::~Msg(){}
int Msg::toIntArray(int * buf){//static buffer
	int i=0;
	buf[i++] = this->dest;
	buf[i++] = this->from;

	switch(this->type){
	case MSG_GRANT:
	case MSG_FREE:
	case MSG_REQUEST:
	case MSG_CANCEL:
		buf[i++] = this->data;
		break;
	default:
		break;

	}

	return i;
}

int Msg::setMsg(int dest, int * buf){

	int i =0;
	this->dest = dest;
	this->from = buf[i++];

	switch(this->type){
	case MSG_GRANT:
	case MSG_FREE:
	case MSG_REQUEST:
	case MSG_CANCEL:
		this->data = buf[i++];
		break;
	default:
		break;
	}

	return i;
}

int Msg::getSize(int msgtype){
	int msgsize;
	switch(msgtype){
	case MSG_GRANT:
	case MSG_FREE:
	case MSG_REQUEST:
	case MSG_CANCEL:
		msgsize = 3;
		break;
	default:
		msgsize = 2;
	}

	return msgsize;
}

int Msg::setType(int type){
	this->type = type;
	this->size = Msg::getSize(type);
}

RequestMsg::RequestMsg(int from, int dest, int type, int data):  Msg(from, dest, type){

	this->data = data;

}

RequestMsg::RequestMsg() : Msg(){

}

RequestMsg::~RequestMsg(){}

int RequestMsg::toIntArray(int * buf){
	int i;
	i = Msg::toIntArray(buf);
	buf[i++] = this->data;


	return i;
}

int RequestMsg::setMsg(int dest, int *buf){
	int i = Msg::setMsg(dest, buf);
	this->data = buf[i++];

}
