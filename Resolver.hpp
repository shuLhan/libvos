/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_RESOLVER_HPP
#define	_LIBVOS_RESOLVER_HPP	1

#include <time.h>
#include "SockAddr.hpp"
#include "DNSQuery.hpp"

using vos::DNSQuery;

namespace vos {

/* DNS QUERY Section */
class Resolver {
public:
	Resolver();
	~Resolver();

	int init();

	void dump();
	int set_server(char *server_list);
	int add_server(char *server_list);

	int create_question_udp(DNSQuery **query, const char *qname);
	int send_query_udp(DNSQuery *question, DNSQuery *answer);
	int send_query_tcp(DNSQuery *question, DNSQuery *answer);
	int send_query(DNSQuery *question, DNSQuery *answer);
	int send_udp(struct sockaddr_in *addr, Buffer *bfr);
	int send_udp_raw(struct sockaddr_in *addr, const char *bfr,
				const int len);

	static unsigned int PORT;
	static unsigned int UDP_PACKET_SIZE;
	static unsigned int TIMEOUT;
	static unsigned int N_TRY;

	Socket		_tcp;
	Socket		_udp;
	SockAddr	*_servers;
private:
	Resolver(const Resolver&);
	void operator=(const Resolver&);
};

}

#endif
