/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_SOCKSERVER_HPP
#define	_LIBVOS_SOCKSERVER_HPP	1

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

	void lock_client();
	void unlock_client();

	int bind(const char* address, const int port);
	int listen(const unsigned int queue_len = 0);
	int bind_listen(const char* address, const int port);

	Socket* accept();
	Socket* accept6();
	Socket* accept_conn();

	void add_client(Socket* client);
	void remove_client(Socket* client);

	struct timeval	_timeout;
	pthread_mutex_t	_client_lock;
	Socket*		_clients;

	static const char* ADDR_WILCARD;
	static const char* ADDR_WILCARD6;
private:
	SockServer(const SockServer&);
	void operator=(const SockServer&);
};

} /* namespace::vos */

#endif
