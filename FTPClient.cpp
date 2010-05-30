/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "FTPClient.hpp"

namespace vos {

FTPClient::FTPClient(Socket *socket) :
	_s(0)
,	_conn_stat(FTP_STT_DISCONNECT)
,	_cmnd()
,	_cmnd_last()
,	_wd()
,	_wd_node(NULL)
,	_sock(socket)
,	_psrv(NULL)
,	_pclt(NULL)
,	_rmsg(NULL)
,	_rmsg_plus(NULL)
,	_next(NULL)
,	_last(this)
{}

FTPClient::~FTPClient()
{
	if (_psrv) {
		delete _psrv;
		_psrv = NULL;
	}
}

void FTPClient::reset()
{
	_cmnd_last.set(&_cmnd);
	_s		= -1;
	_rmsg		= NULL;
	_rmsg_plus	= NULL;
}

void FTPClient::reply()
{
	printf("[FTPD] reply : %s %s\n", _rmsg, _rmsg_plus);

	_sock->reset();
	_sock->writes(_rmsg, _rmsg_plus);
	_sock->send_raw(0);
}

void FTPClient::reply_raw(int code, const char* msg, const char* msg_add)
{
	_s		= code;
	_rmsg		= msg;
	_rmsg_plus	= msg_add;
	reply();
}

void FTPClient::ADD(FTPClient **list, FTPClient *client)
{
	if (!(*list)) {
		(*list) = client;
	} else {
		(*list)->_last->_next = client;
	}
	(*list)->_last = client;
}

void FTPClient::REMOVE(FTPClient **list, FTPClient *client)
{
	FTPClient *last = NULL;
	FTPClient *curr = (*list);

	while (curr != client) {
		last = curr;
		curr = curr->_next;
	}
	if (!curr) {
		return;
	}

	if (!last) {
		(*list) = (*list)->_next;
		if ((*list)) {
			last = (*list);
			while (last->_next) {
				last = last->_next;
			}
			(*list)->_last = last;
		}
	} else {
		last->_next = curr->_next;
		if ((*list)->_last == curr) {
			(*list)->_last = last;
		}
	}

	curr->_next = NULL;
	delete curr;
}

} /* namespace::vos */
