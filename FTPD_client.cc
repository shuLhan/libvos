//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "FTPD_client.hh"

namespace vos {

/**
 * @method		: FTPD_client::FTPD_client
 * @param		:
 *	> socket	: pointer to client connection.
 * @desc		: FTPD_client constructor.
 */
FTPD_client::FTPD_client(Socket *socket) :
	_s(0)
,	_conn_stat(FTP_STT_DISCONNECT)
,	_cmd()
,	_cmd_last()
,	_wd()
,	_wd_node(NULL)
,	_path()
,	_path_base()
,	_path_real()
,	_path_node(NULL)
,	_sock(socket)
,	_psrv(NULL)
,	_pclt(NULL)
,	_rmsg(NULL)
,	_rmsg_plus(NULL)
,	_next(NULL)
,	_prev(NULL)
,	_last(this)
{}

/**
 * @method	: FTPD_client::~FTPD_client
 * @desc	: FTPD_client destructor.
 */
FTPD_client::~FTPD_client()
{
	if (_psrv) {
		delete _psrv;
		_psrv = NULL;
	}
	_next = NULL;
	_prev = NULL;
	_last = NULL;
}

/**
 * @method	: FTPD_client::reset
 * @desc	: reset some attributes of this object.
 */
void FTPD_client::reset()
{
	_cmd_last.set(&_cmd);
	_s		= -1;
	_rmsg		= NULL;
	_rmsg_plus	= NULL;
}

/**
 * @method	: FTPD_client::reply
 * @return	:
 *	< >=0	: success.
 *	< -1	: fail.
 * @desc	: Send a reply to this FTPD client object. Reply code and
 * their messages is taken from '_rmsg' and '_rmsg_plus'.
 */
int FTPD_client::reply()
{
	if (LIBVOS_DEBUG) {
		printf(	"[vos::FTPD_clt] reply    : %s"
			"                add. msg : %s\n"
			, _rmsg?_rmsg:"-", _rmsg_plus?_rmsg_plus:"-");
	}

	_sock->reset();
	return _sock->writes(_rmsg, _rmsg_plus);
}

/**
 * @method		: FTPD_client::reply_raw
 * @param		:
 *	> code		: reply code.
 *	> msg		: reply message.
 *	> msg_add	: additional reply message.
 * @desc		: Send a reply 'msg' plus 'msg_add' to this FTPD
 * client object.
 */
int FTPD_client::reply_raw(int code, const char* msg, const char* msg_add)
{
	_s		= code;
	_rmsg		= msg;
	_rmsg_plus	= msg_add;
	return reply();
}

/**
 * @method		: FTPD_client::ADD
 * @param		:
 *	> list		: list of FTPD_client connection.
 *	> client	: pointer to a new FTPD_client object.
 * @desc		: add 'client' to the 'list' of connection.
 */
void FTPD_client::ADD(FTPD_client** list, FTPD_client* client)
{
	if (!client) {
		return;
	}
	if (!(*list)) {
		(*list) = client;
	} else {
		(*list)->_last->_next	= client;
		client->_prev		= (*list)->_last;
		(*list)->_last		= client;
	}
}

/**
 * @method		: FTPD_client::REMOVE
 * @param		:
 *	> list		: list of FTPD_client connection.
 * 	> client	: pointer to FTPD_client object that will be removed.
 * @desc		: Remove 'client' from the 'list', and delete the
 * client object from memory even if client is not found in the list.
 */
void FTPD_client::REMOVE(FTPD_client** list, FTPD_client* client)
{
	if (!(*list) || !client) {
		return;
	}

	if ((*list) == client) {
		(*list) = client->_next;
		if ((*list)) {
			(*list)->_last = client->_last;
			(*list)->_prev = NULL;
		}
	} else if ((*list)->_last == client) {
		(*list)->_last		= client->_prev;
		(*list)->_last->_next	= NULL;
	} else {
		FTPD_client* p = (*list);

		/* make sure client is from the same list */
		while (p && p->_next != client) {
			p = p->_next;
		}
		if (p) {
			p->_next		= client->_next;
			client->_next->_prev	= p;
		}
	}

	delete client;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
