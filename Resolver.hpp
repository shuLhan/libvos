/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_RESOLVER_HPP
#define	_LIBVOS_RESOLVER_HPP	1

#include <time.h>
#include "DNSQuery.hpp"
#include "SockServer.hpp"

namespace vos {

/**
 * @class			: Resolver
 * @attr			:
 *	- _tcp			: Socket object for handling TCP connection.
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
 *	to one of the server in the list of '_servers' using UDP or TCP
 *	protocol.
 */
class Resolver : public Socket {
public:
	Resolver();
	~Resolver();

	int init();
	void dump();
	int set_server(char* server_list);
	int add_server(char* server_list);

	int send_query_udp(DNSQuery* question, DNSQuery* answer);
	int send_query_tcp(DNSQuery* question, DNSQuery* answer);
	int send_query(DNSQuery* question, DNSQuery* answer);

	Socket		_tcp;
	SockAddr*	_servers;

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
