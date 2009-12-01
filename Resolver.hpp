/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_RESOLVER_HPP
#define	_LIBVOS_RESOLVER_HPP	1

#include <time.h>
#include "Record.hpp"
#include "DNSQuery.hpp"

using vos::DNSQuery;

namespace vos {

#define	DNS_DEF_PORT		53
#define	DNS_UDP_PACKET_SIZE	512
#define	RESOLVER_DEF_TIMEOUT	10
#define	RESOLVER_DEF_TO_TRY	0

/* DNS QUERY Section */
class Resolver {
public:
	Resolver();
	~Resolver();

	int init();

	void dump();
	void set_server(const char *server_list);
	void add_server(const char *server_list);

	int create_question_udp(DNSQuery **query, const char *qname);
	int send_query_udp(DNSQuery *question, DNSQuery *answer);
	int send_query_tcp(DNSQuery *question, DNSQuery **answer);

	Socket		_tcp;
	Socket		_udp;
	Record		*_servers;
private:
	DISALLOW_COPY_AND_ASSIGN(Resolver);
};

}

#endif
