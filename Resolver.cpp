/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Resolver.hpp"

namespace vos {

Resolver::Resolver() :
	_tcp(),
	_udp(),
	_servers(NULL)
{}

Resolver::~Resolver()
{
	if (_servers) {
		delete _servers;
		_servers = NULL;
	}
}


/**
 * @desc	: initialize Resolver object.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Resolver::init()
{
	int s;

	srand(time(NULL));

	s = _tcp.init(Socket::DFLT_BUFFER_SIZE);
	if (s < 0)
		return s;

	s = _udp.init(DNS_UDP_PACKET_SIZE);
	if (s < 0)
		return s;
	
	_tcp.create_tcp();
	_udp.create_udp();

	FD_SET(_udp._d, &_udp._readfds);
	FD_SET(_tcp._d, &_tcp._readfds);

	return 0;
}

void Resolver::dump()
{
	if (_servers) {
		printf("; Servers\n");
		_servers->dump();
	}
}

void Resolver::set_server(const char *server_list)
{
	if (_servers) {
		delete _servers;
		_servers = NULL;
	}

	add_server(server_list);
}

void Resolver::add_server(const char *server_list)
{
	Record *server = new Record();

	while (*server_list) {
		if (*server_list == ',') {
			Record::ADD_COL(&_servers, server);
			server = new Record();
		} else {
			if (isalnum(*server_list) || *server_list == '.') {
				server->appendc(*server_list);
			}
		}
		*server_list++;
	}

	Record::ADD_COL(&_servers, server);
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Resolver::create_question_udp(DNSQuery **query, const char *qname)
{
	int	s;
	int	len;
	Buffer	label;
	Buffer	*q	= NULL;

	if (!qname)
		return 0;

	s = label.init(NULL);
	if (s < 0)
		return s;

	if (!(*query)) {
		s = DNSQuery::INIT(query, NULL);
		if (s != 0) {
			return s;
		}
	} else {
		(*query)->reset(vos::DNSQ_DO_ALL);
	}

	(*query)->_id		= htons(rand() % 65536);
	(*query)->_flag		= htons(HDR_IS_QUERY | OPCODE_QUERY | RTYPE_RD);
	(*query)->_n_qry	= htons(1);
	(*query)->_n_ans	= htons(0);
	(*query)->_n_aut	= htons(0);
	(*query)->_n_add	= htons(0);
	(*query)->_type		= htons(QUERY_T_ADDRESS);
	(*query)->_class	= htons(QUERY_C_IN);
	(*query)->_name.copy_raw(qname, 0);

	len = (*query)->_name._i + 16;

	if (! (*query)->_bfr) {
		(*query)->_bfr = new Buffer();
	}
	q = (*query)->_bfr;
	q->reset();

	if (len > q->_l)
		q->resize(len);

	memcpy(q->_v, (*query), DNS_HEADER_SIZE);
	q->_i = DNS_HEADER_SIZE;

	while (*qname) {
		if (*qname == '.') {
			if (label._i) {
				len = q->_i + label._i + 1;
				if (len > q->_l)
					q->resize(len);

				q->_v[q->_i] = label._i;
				q->_i++;
				q->append(&label);
			}
			label.reset();
		} else {
			label.appendc(*qname);
		}
		*qname++;
	}

	if (label._i) {
		len = q->_i + label._i + 1;
		if (len > q->_l)
			q->resize(len);

		q->_v[q->_i] = label._i;
		q->_i++;
		q->append(&label);
	}

	/* end of query */
	q->_v[q->_i] = 0;
	q->_i++;

	len = q->_i + 4;
	if (len > q->_l)
		q->resize(len);

	memcpy(&q->_v[q->_i], &(*query)->_type, 2);
	q->_i += 2;
	memcpy(&q->_v[q->_i], &(*query)->_class, 2);
	q->_i += 2;

	(*query)->ntohs();

	return 0;
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int Resolver::send_query_udp(DNSQuery *question, DNSQuery *answer)
{
	int	s	= 0;
	int	n_try	= 0;
	int	maxfd	= 0;
	fd_set	allfds;
	Record	*server	= _servers;

	if (!question)
		return 0;

	FD_ZERO(&allfds);
	FD_ZERO(&_udp._readfds);
	FD_SET(_udp._d, &allfds);
	maxfd = _udp._d + 1;

	while (server) {
		n_try = 0;
		_udp.reset();

		if (LIBVOS_DEBUG) {
			printf(">> querying %s... ", server->_v);
		}

		s = _udp.connect_to(server->_v, DNS_DEF_PORT);
		if (s < 0) {
			server = server->_next_col;
			continue;
		}

		s = _udp.send(question->_bfr);
		if (s < 0) {
			server = server->_next_col;
			continue;
		}

		do {
			_udp._readfds		= allfds;
			_udp._timeout.tv_sec	= RESOLVER_DEF_TIMEOUT;
			_udp._timeout.tv_usec	= 0;

			select(maxfd, &_udp._readfds, NULL, NULL,
				&_udp._timeout);
	
			if (FD_ISSET(_udp._d, &_udp._readfds)) {
				s = _udp.read();
				if (s <= 0) {
					return s;
				}

				answer->reset(DNSQ_DO_ALL);
				answer->extract(&_udp, BUFFER_IS_UDP);

				if (answer->_n_ans <= 0) {
					break;
				}
				if (question->_id != answer->_id) {
					break;
				}
				s = question->_name.like(&answer->_name);
				if (s != 0) {
					break;
				}
				if (LIBVOS_DEBUG) {
					printf(" OK\n");
				}
				return 0;
			} else {
				++n_try;

				if (LIBVOS_DEBUG) {
					printf(" timeout...(%d)\n", n_try);
				}
			}
		} while (n_try < RESOLVER_DEF_TO_TRY);

		server = server->_next_col;
	}

	return -E_SOCK_TIMEOUT;
}

int Resolver::send_query_tcp(DNSQuery *question, DNSQuery **answer)
{
	int	s	= 0;
	int	n_try	= 0;
	Record	*server	= _servers;

	if (!question)
		return 0;

	while (server) {
		n_try = 0;
		_tcp.reset();

		printf(">> querying %s...\n", server->_v);
		s = _tcp.connect_to(server->_v, DNS_DEF_PORT);
		if (s < 0) {
			server = server->_next_col;
			continue;
		}

		_tcp.send(question->_bfr);

		do {
			_tcp._timeout.tv_sec	= RESOLVER_DEF_TIMEOUT;
			_tcp._timeout.tv_usec	= 0;

			select(_tcp._d + 1, &_tcp._readfds, NULL, NULL,
				&_tcp._timeout);
	
			if (FD_ISSET(_tcp._d, &_tcp._readfds)) {
				s = _tcp.read();

				if (! (*answer)) {
					(*answer) = new DNSQuery();
				} else {
					(*answer)->reset(DNSQ_DO_ALL);
				}

				(*answer)->extract(&_tcp, BUFFER_IS_UDP);

				return 0;
			} else {
				++n_try;
				printf(">> timeout...(%d)\n", n_try);
			}
		} while (n_try < RESOLVER_DEF_TO_TRY);

		server = server->_next_col;
	}

	return -E_SOCK_TIMEOUT;
}

} /* namespace::vos */
