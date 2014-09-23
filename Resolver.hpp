/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_RESOLVER_HPP
#define	_LIBVOS_RESOLVER_HPP	1

#include <time.h>
#include "DNSQuery.hpp"
#include "Socket.hpp"

namespace vos {

/**
 * @class			: Resolver
 * @attr			:
 *	- _maxfd		: maximum descriptor in this object.
 *	- _fd_all		: collection of all open descriptor.
 *	- _fd_read		: temporary collection.
 *	- _n_try		: temporary counter.
 *	- _timeout		: temporary data for storing timeout value.
 *	- _servers		: list of parent DNS server addresses.
 *
 *	- PORT			: static, default DNS server port.
 *	- UDP_PACKET_SIZE	: static, default DNS packet size.
 *	- TIMEOUT		:
 *		static, default time-out value in second for waiting
 *		reply from server.
 *	- N_TRY			:
 *		static, maximum number of client trying to resend a packet
 *		back to server when no reply from server after specific
 *		'TIMEOUT' is reached.
 * @desc			:
 *
 *	module for translating hostname to IP address, by sending a DNS query
 *	to one of the server in the list of '_servers' using UDP or TCP.
 */
class Resolver : public Socket {
public:
	Resolver();
	~Resolver();

	int init(const int type = SOCK_DGRAM);
	void dump();
	int set_server(char* server_list);
	int add_server(char* server_list);
	void rotate_server();

	int send_udp(DNSQuery* question);
	int recv_udp(DNSQuery* answer);

	int send_tcp(DNSQuery* question);
	int recv_tcp(DNSQuery* answer);

	int resolve_udp(DNSQuery* question, DNSQuery* answer);
	int resolve_tcp(DNSQuery* question, DNSQuery* answer);
	int resolve(DNSQuery* question, DNSQuery* answer);

	int		_maxfd;
	fd_set		_fd_all;
	fd_set		_fd_read;
	unsigned int	_n_try;
	struct timeval	_timeout;
	SockAddr*	_servers;
	SockAddr*	_p_server;

	static unsigned int PORT;
	static unsigned int UDP_PACKET_SIZE;
	static unsigned int TIMEOUT;
	static unsigned int N_TRY;
private:
	Resolver(const Resolver&);
	void operator=(const Resolver&);
};

}

#endif
