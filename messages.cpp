/*
 * messages.cpp
 *
 *  Created on: 2009-05-28
 *      Author: chriss, tzok
 */
#include "def.h"

#include <stdio.h>

Msg::Msg() {

}

Msg::Msg(int from, int dest, int type) {
	this->dest = dest;
	this->from = from;
	this->setType(type);
}
Msg::~Msg() {
}
int Msg::toIntArray(int * buf) {//static buffer
	int i = 0;
	buf[i++] = this->dest;
	buf[i++] = this->from;
	buf[i++] = this->timestamp;

	switch (this->type) {
	case MSG_GRANT:
	case MSG_FREE:
	case MSG_REQUEST:
	case MSG_CANCEL:
		buf[i++] = this->data;
		break;
	case MSG_ECHO:
	case MSG_FLOOD:
	case MSG_SHORT:
		buf[i++] = (int)(this->weight*PRECISION);
		break;
	default:
		break;

	}

	return i;
}

int Msg::setMsg(int dest, int * buf) {

	int i = 0;
	this->dest = dest;
	this->from = buf[i++];
	this->timestamp = buf[i++];
	switch (this->type) {
	case MSG_GRANT:
	case MSG_FREE:
	case MSG_REQUEST:
	case MSG_CANCEL:
		this->data = buf[i++];
		break;
	case MSG_ECHO:
	case MSG_FLOOD:
	case MSG_SHORT:
		this->weight = (float)buf[i++]/PRECISION;
		break;
	default:
		break;
	}

	return i;
}

int Msg::getSize(int msgtype) {
	int msgsize;
	switch (msgtype) {
	case MSG_GRANT:
	case MSG_FREE:
	case MSG_REQUEST:
	case MSG_CANCEL:
	case MSG_FLOOD:
	case MSG_ECHO:
	case MSG_SHORT:
		msgsize = 4;
		break;



	default:
		msgsize = 3;
	}

	return msgsize;
}

int Msg::setType(int type) {
	this->type = type;
	this->size = Msg::getSize(type);
}


