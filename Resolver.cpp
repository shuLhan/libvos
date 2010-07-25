/*
 * Copyright (C) 2009 kilabit.org
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
Resolver::Resolver() :
	_tcp(),
	_udp(),
	_servers(NULL)
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
 *	< <0	: fail.
 * @desc	: initialize Resolver object.
 */
int Resolver::init()
{
	int s;

	s = _tcp.init(Socket::DFLT_BUFFER_SIZE);
	if (s != 0)
		return s;

	s = _udp.init(UDP_PACKET_SIZE);
	if (s != 0)
		return s;

	s = _tcp.create_tcp();
	if (s != 0)
		return s;

	s = _udp.create_udp();

	return s;
}

/**
 * @method	: Resolver::dump
 * @desc	: print content of this Resolver object.
 */
void Resolver::dump()
{
	if (_servers) {
		printf("; Servers\n");
		_servers->dump();
	}
}

/**
 * @method		: Resolver::set_server
 * @param		:
 *	> server_list	: list of server name, separated by comma.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: add another server to list of server '_servers'.
 */
int Resolver::set_server(char *server_list)
{
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
 *	< <0		: fail.
 * @desc		: add another server to list of server '_servers'.
 */
int Resolver::add_server(char *server_list)
{
	register int	s;
	char		*addr;
	SockAddr	*saddr;

	addr = server_list; 
	while (*server_list) {
		if (*server_list == ',') {
			*server_list = '\0';
			s = SockAddr::INIT(&saddr, addr, PORT);
			*server_list = ',';
			if (s < 0) {
				return s;
			}
			SockAddr::ADD(&_servers, saddr);

			server_list++;
			addr = server_list;
		} else {
			server_list++;
		}
	}
	s = SockAddr::INIT(&saddr, addr, PORT);
	if (s < 0) {
		return s;
	}
	SockAddr::ADD(&_servers, saddr);

	return 0;
}

/**
 * @method	: Resolver::create_question_udp
 * @param	:
 *	> query	: return value.
 *	> qname	: hostname that will be saved in DNSQuery object.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: create a DNSQuery object for hostname 'qname'.
 */
int Resolver::create_question_udp(DNSQuery **query, const char *qname)
{
	int	s;
	int	len;
	Buffer	label;
	Buffer	*q	= NULL;

	if (!qname)
		return 0;

	if (!(*query)) {
		s = DNSQuery::INIT(query, NULL, BUFFER_IS_UDP);
		if (s != 0) {
			return s;
		}
	} else {
		(*query)->reset(vos::DNSQ_DO_ALL);
	}

	(*query)->_id		= htons((uint16_t) (rand() % 65536));
	(*query)->_flag		= htons(HDR_IS_QUERY | OPCODE_QUERY | RTYPE_RD);
	(*query)->_n_qry	= htons(1);
	(*query)->_n_ans	= htons(0);
	(*query)->_n_aut	= htons(0);
	(*query)->_n_add	= htons(0);
	(*query)->_type		= htons(QUERY_T_ADDRESS);
	(*query)->_class	= htons(QUERY_C_IN);
	(*query)->_name.copy_raw(qname);

	len = (*query)->_name._i + 16;

	if (! (*query)->_bfr) {
		(*query)->_bfr = new Buffer();
	}
	q = (*query)->_bfr;
	q->reset();

	if (len > q->_l)
		q->resize(len);

	memcpy(q->_v, (*query), DNS_HDR_SIZE);
	q->_i = DNS_HDR_SIZE;

	while (*qname) {
		if (*qname == '.') {
			if (label._i) {
				len = q->_i + label._i + 1;
				if (len > q->_l)
					q->resize(len);

				q->_v[q->_i] = (char) label._i;
				q->_i++;
				q->append(&label);
			}
			label.reset();
		} else {
			label.appendc(*qname);
		}
		qname++;
	}

	if (label._i) {
		len = q->_i + label._i + 1;
		if (len > q->_l)
			q->resize(len);

		q->_v[q->_i] = (char) label._i;
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

	(*query)->net_to_host();

	return 0;
}

/**
 * @method		: Resolver::send_query_udp
 * @param		:
 *	> question	: query to be send to server.
 *	> answer	: return value, answer from server.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		:
 *	send 'question' to server to get an 'answer' using UDP protocol.
 */
int Resolver::send_query_udp(DNSQuery *question, DNSQuery *answer)
{
	int		s	= 0;
	int		maxfd	= 0;
	unsigned int	n_try	= 0;
	fd_set		fd_all;
	fd_set		fd_read;
	SockAddr	*server	= _servers;

	if (!question)
		return 0;

	FD_ZERO(&fd_all);
	FD_ZERO(&fd_read);
	FD_SET(_udp._d, &fd_all);
	maxfd = _udp._d + 1;

	while (server) {
		n_try = 0;
		_udp.reset();

		if (LIBVOS_DEBUG) {
			printf(">> querying %s... ", server->_addr->_v);
		}

		s = (int) _udp.send_udp(server->_in, question->_bfr);
		if (s < 0) {
			server = server->_next;
			continue;
		}

		do {
			fd_read			= fd_all;
			_udp._timeout.tv_sec	= TIMEOUT;
			_udp._timeout.tv_usec	= 0;

			s = select(maxfd, &fd_read, NULL, NULL, &_udp._timeout);
			if (s < 0) {
				if (EINTR == errno) {
					goto intr;
				}
			}
			if (0 == s || 0 == FD_ISSET(_udp._d, &fd_read)) {
				++n_try;
				if (LIBVOS_DEBUG) {
					printf(">> timeout...(%d)\n", n_try);
				}
				continue;
			}

			s = _udp.read();
			if (s <= 0) {
				return s;
			}

			answer->reset(DNSQ_DO_ALL);
			answer->set_buffer(&_udp, BUFFER_IS_UDP);
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
			if (LIBVOS_DEBUG) {
				printf(" OK\n");
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
 *	< <0		: fail.
 * @desc		:
 *	send 'question' to server to get an 'answer' using TCP protocol.
 */
int Resolver::send_query_tcp(DNSQuery *question, DNSQuery *answer)
{
	int		s	= 0;
	unsigned int	n_try	= 0;
	fd_set		fd_all;
	fd_set		fd_read;
	SockAddr	*server	= _servers;

	if (!question)
		return 0;

	FD_ZERO(&fd_all);
	FD_ZERO(&fd_read);
	FD_SET(_tcp._d, &fd_all);

	while (server) {
		n_try = 0;
		_tcp.reset();

		if (LIBVOS_DEBUG) {
			printf(">> querying %s...\n", server->_addr->_v);
		}

		s = _tcp.connect_to(server->_in);
		if (s < 0) {
			server = server->_next;
			continue;
		}

		s = _tcp.send(question->_bfr);
		if (s < 0) {
			server = server->_next;
			continue;
		}

		do {
			fd_read			= fd_all;
			_tcp._timeout.tv_sec	= TIMEOUT;
			_tcp._timeout.tv_usec	= 0;

			s = select(_tcp._d + 1, &fd_read, NULL, NULL,
					&_tcp._timeout);
			if (s < 0) {
				if (EINTR == errno) {
					goto intr;
				}
			}
			if (0 == s || !FD_ISSET(_tcp._d, &fd_read)) {
				++n_try;
				if (LIBVOS_DEBUG) {
					printf(">> timeout...(%d)\n", n_try);
				}
				continue;
			}

			s = _tcp.read();
			if (s <= 0) {
				return s;
			}

			answer->reset(DNSQ_DO_ALL);
			answer->set_buffer(&_tcp, BUFFER_IS_TCP);
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
			if (LIBVOS_DEBUG) {
				printf(" OK\n");
			}
			return 0;
		} while (n_try < N_TRY);

		server = server->_next;
	}
intr:
	return -1;
}

/**
 * @method		: Resolver::send_query
 * @param		:
 *	> question	: query to be send to server.
 *	> answer	: return value, answer from server.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		:
 *	send 'question' to server to get an 'answer'. This is a generic form
 *	of sending query, query send using protocol based on type of buffer in
 *	'question'.
 */
int Resolver::send_query(DNSQuery *question, DNSQuery *answer)
{
	int s;

	if (!question)
		return 0;

	if (question->_bfr_type == BUFFER_IS_TCP) {
		s = send_query_tcp(question, answer);
	} else {
		s = send_query_udp(question, answer);
	}

	return s;
}

/**
 * @method	: Resolver::send_udp
 * @param	:
 *	> addr	: address of end point where 'bfr' will be send.
 *	> bfr	: data to be send.
 * @return	:
 *	< >=0	: success, number of bytes sended.
 *	< <0	: fail.
 * @desc	: send data 'bfr' to 'addr' using UDP protocol.
 */
long int Resolver::send_udp(struct sockaddr_in *addr, Buffer *bfr)
{
	return _udp.send_udp(addr, bfr);
}

/**
 * @method	: Resolver::send_udp_raw
 * @param	:
 *	> addr	: address of end point where 'bfr' will be send.
 *	> bfr	: data to be send.
 *	> len	: length of 'bfr' data.
 * @return	:
 *	< >=0	: success, number of bytes sended.
 *	< <0	: fail.
 * @desc	: send data 'bfr' to 'addr' using UDP protocol.
 */
long int Resolver::send_udp_raw(struct sockaddr_in *addr, const char *bfr,
				const int len)
{
	return _udp.send_udp_raw(addr, bfr, len);
}

} /* namespace::vos */
