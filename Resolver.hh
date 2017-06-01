//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_RESOLVER_HH
#define _LIBVOS_RESOLVER_HH 1

#include <sys/select.h>
#include <time.h>

#include "ListBuffer.hh"
#include "DNSQuery.hh"
#include "Socket.hh"
#include "ListSockAddr.hh"

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

	int is_servers_set();
	int init(const int type = SOCK_DGRAM);
	void servers_reset();
	int set_server(const char* server_list);
	int add_server(const char* server_list);

	int send_udp(DNSQuery* question);
	int recv_udp(DNSQuery* answer);

	int send_tcp(DNSQuery* question);
	int recv_tcp(DNSQuery* answer);

	int resolve_udp(DNSQuery* question, DNSQuery* answer);
	int resolve_tcp(DNSQuery* question, DNSQuery* answer);
	int resolve(DNSQuery* question, DNSQuery* answer);

	const char* chars();

	int		_maxfd;
	fd_set		_fd_all;
	fd_set		_fd_read;
	unsigned int	_n_try;
	struct timeval	_timeout;
	ListSockAddr	*_servers;
	static uint16_t PORT;
	static unsigned int UDP_PACKET_SIZE;
	static unsigned int TIMEOUT;
	static unsigned int N_TRY;

	static const char* __cname;
private:
	Resolver(const Resolver&);
	void operator=(const Resolver&);
};

}

#endif
// vi: ts=8 sw=8 tw=78:
