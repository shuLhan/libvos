//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_SOCKSERVER_HH
#define _LIBVOS_SOCKSERVER_HH 1

#include <sys/time.h>

#include "Locker.hh"
#include "Socket.hh"

namespace vos {

/**
 * @class			: SockServer
 * @attr			:
 *	- _timeout		: time data used by socket as server.
 *	- _client_lock		: lock for accessing list of clients object.
 *	- _clients		: list of client connections.
 *	- ADDR_WILCARD		: static, wilcard address for IPv4.
 *	- ADDR_WILCARD6		: static, wilcard address for IPv6.
 * @desc			:
 * Module for creating socket as server.
 */
class SockServer : public Socket {
public:
	SockServer();
	~SockServer();

	int bind(const char* address, const uint16_t port);
	int listen(const unsigned int queue_len = 0);
	int bind_listen(const char* address, const uint16_t port);

	Socket* accept();
	Socket* accept6();
	Socket* accept_conn();

	void add_client(Socket* client);
	void remove_client(Socket* client);

	struct timeval	_timeout;
	List*		_clients;

	static const char* ADDR_WILCARD;
	static const char* ADDR_WILCARD6;

	static const char* __cname;
private:
	SockServer(const SockServer&);
	void operator=(const SockServer&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
