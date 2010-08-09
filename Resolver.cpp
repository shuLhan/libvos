/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Resolver.hpp"

namespace vos {

unsigned int Resolver::PORT		= 53;
unsigned int Resolver::UDP_PACKET_SIZE	= 512;
unsigned int Resolver::TIMEOUT		= 6;
unsigned int Resolver::N_TRY		= 0;

/**
 * @method	: Resolver::Resolver
 * @desc	:
 *	Resolver object constructor.
 *	This constructor initialize RNG for DNS transaction ID.
 */
Resolver::Resolver() : Socket()
,	_tcp()
,	_servers(NULL)
{
	srand((unsigned int) time(NULL));
}

/**
 * @method	: Resolver::~Resolver
 * @desc	: Resolver object destructor.
 */
Resolver::~Resolver()
{
	if (_servers) {
		delete _servers;
		_servers = NULL;
	}
}

/**
 * @method	: Resolver::init
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: initialize Resolver object.
 */
int Resolver::init()
{
	int s;

	s = _tcp.create();
	if (s != 0) {
		return -1;
	}

	s = create_udp();

	return s;
}

/**
 * @method	: Resolver::dump
 * @desc	: print content of this Resolver object.
 */
void Resolver::dump()
{
	if (_servers) {
		printf("\n[vos::Resolver] dump:\n; Servers\n");
		_servers->dump();
	}
}

/**
 * @method		: Resolver::set_server
 * @param		:
 *	> server_list	: list of server name, separated by comma.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: create a new parent server list.
 */
int Resolver::set_server(char* server_list)
{
	if (!server_list) {
		return 0;
	}
	if (_servers) {
		delete _servers;
		_servers = NULL;
	}

	return add_server(server_list);
}

/**
 * @method		: Resolver::add_server
 * @param		:
 *	> server_list	: list of server name, separated by comma.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: add another server to list of server '_servers'.
 */
int Resolver::add_server(char* server_list)
{
	if (!server_list) {
		return 0;
	}

	register int	s;
	Buffer		addr;
	char*		p;
	SockAddr*	saddr;

	p = server_list;

	while (*p) {
		while (isspace(*p)) {
			p++;
		}
		while (*p && (isalnum(*p) || *p == '.')) {
			addr.appendc(*p);
			p++;
		}
		while (isspace(*p)) {
			p++;
		}
		if (!*p) {
			break;
		}
		if (*p != ',') {
			return -1;
		}
		p++;
		if (!addr.is_empty()) {
			s = SockAddr::INIT(&saddr, AF_INET, addr._v, PORT);
			if (s < 0) {
				return -1;
			}
			SockAddr::ADD(&_servers, saddr);
			addr.reset();
		}
	}
	if (!addr.is_empty()) {
		s = SockAddr::INIT(&saddr, AF_INET, addr._v, PORT);
		if (s < 0) {
			return -1;
		}
		SockAddr::ADD(&_servers, saddr);
	}

	return 0;
}

/**
 * @method		: Resolver::send_query_udp
 * @param		:
 *	> question	: query to be send to server.
 *	> answer	: return value, answer from server.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	send 'question' to server to get an 'answer' using UDP protocol.
 */
int Resolver::send_query_udp(DNSQuery* question, DNSQuery* answer)
{
	if (!question) {
		return 0;
	}

	int		s	= 0;
	int		maxfd	= 0;
	unsigned int	n_try	= 0;
	fd_set		fd_all;
	fd_set		fd_read;
	struct timeval	timeout;
	SockAddr*	server	= _servers;

	if (question->_bfr_type == BUFFER_IS_TCP) {
		s = question->to_udp();
		if (s < 0) {
			return -1;
		}
	}

	FD_ZERO(&fd_all);
	FD_ZERO(&fd_read);
	FD_SET(_d, &fd_all);
	maxfd = _d + 1;

	while (server) {
		if (LIBVOS_DEBUG) {
			printf("[vos::Resolver] >> querying %s ...\n"
				, server->v());
		}

		n_try = 0;
		reset();

		s = (int) send_udp(&server->_in, question);
		if (s < 0) {
			server = server->_next;
			continue;
		}

		do {
			fd_read		= fd_all;
			timeout.tv_sec	= TIMEOUT;
			timeout.tv_usec	= 0;

			s = select(maxfd, &fd_read, NULL, NULL, &timeout);
			if (s < 0) {
				if (EINTR == errno) {
					goto intr;
				}
			}
			if (0 == s || 0 == FD_ISSET(_d, &fd_read)) {
				++n_try;
				if (LIBVOS_DEBUG) {
					printf(
					"[vos::Resolver] >> timeout...(%d)\n"
					, n_try);
				}
				continue;
			}

			s = (int) recv_udp(&server->_in);
			if (s <= 0) {
				return s;
			}

			answer->reset(DNSQ_DO_ALL);
			answer->set(this);
			answer->extract_header();
			answer->extract_question();

			if ((answer->_flag & RCODE_FLAG) != 0) {
				break;
			}
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
			return 0;
		} while (n_try < N_TRY);

		server = server->_next;
	}

intr:
	return -1;
}

/**
 * @method		: Resolver::send_query_tcp
 * @param		:
 *	> question	: query to be send to server.
 *	> answer	: return value, answer from server.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	send 'question' to server to get an 'answer' using TCP protocol.
 */
int Resolver::send_query_tcp(DNSQuery* question, DNSQuery* answer)
{
	if (!question) {
		return -1;
	}

	int		s	= 0;
	unsigned int	n_try	= 0;
	fd_set		fd_all;
	fd_set		fd_read;
	struct timeval	timeout;
	SockAddr*	server	= _servers;

	if (question->_bfr_type == BUFFER_IS_UDP) {
		s = question->to_tcp();
		if (s < 0) {
			return -1;
		}
	}

	FD_ZERO(&fd_all);
	FD_ZERO(&fd_read);
	FD_SET(_tcp._d, &fd_all);

	while (server) {
		if (LIBVOS_DEBUG) {
			printf("[vos::Resolver] >> querying %s...\n"
				, server->v());
		}

		n_try = 0;
		_tcp.reset();

		s = _tcp.connect_to(&server->_in);
		if (s < 0) {
			server = server->_next;
			continue;
		}

		s = _tcp.write(question);
		if (s < 0) {
			server = server->_next;
			continue;
		}

		do {
			fd_read		= fd_all;
			timeout.tv_sec	= TIMEOUT;
			timeout.tv_usec	= 0;

			s = select(_tcp._d + 1, &fd_read, NULL, NULL
					, &timeout);
			if (s < 0) {
				if (EINTR == errno) {
					return -1;
				}
			}
			if (0 == s || !FD_ISSET(_tcp._d, &fd_read)) {
				++n_try;
				if (LIBVOS_DEBUG) {
					printf(
					"[vos::Resolver] >> timeout...(%d)\n"
					, n_try);
				}
				continue;
			}

			s = _tcp.read();
			if (s <= 0) {
				return -1;
			}

			answer->reset(DNSQ_DO_ALL);
			answer->to_udp(&_tcp);
			answer->extract_header();
			answer->extract_question();

			if ((answer->_flag & RCODE_FLAG) != 0) {
				break;
			}
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
			return 0;
		} while (n_try < N_TRY);

		server = server->_next;
	}
	return -1;
}

/**
 * @method		: Resolver::send_query
 * @param		:
 *	> question	: query to be send to server.
 *	> answer	: return value, answer from server.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	send 'question' to server to get an 'answer'. This is a generic form
 *	of sending query, query send using protocol based on type of buffer in
 *	'question'.
 */
int Resolver::send_query(DNSQuery* question, DNSQuery* answer)
{
	if (!question) {
		return 0;
	}

	register int s;

	if (question->_bfr_type == BUFFER_IS_TCP) {
		s = send_query_tcp(question, answer);
	} else {
		s = send_query_udp(question, answer);
	}

	return s;
}

} /* namespace::vos */
