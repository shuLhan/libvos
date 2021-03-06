//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Resolver.hh"

namespace vos {

const char* Resolver::__cname = "Resolver";

uint16_t Resolver::PORT = 53;
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
,	_maxfd(0)
,	_fd_all()
,	_fd_read()
,	_n_try(0)
,	_timeout()
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
	}
}

/**
 * Method `is_servers_set()` will return 1 if list of servers contain one
 * address. If its empty, no address added, it will return 0.
 */
int Resolver::is_servers_set()
{
	if (_servers && _servers->size() > 0) {
		return 1;
	}
	return 0;
}

/**
 * @method	: Resolver::init
 * @type	:
 *	> type	: connection type, valid values:
 *			- SOCK_STREAM for TCP, and
 *			- SOCK_DGRAM for UDP
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: initialize Resolver object.
 *
 * This assume that the server address list _servers already populated using
 * add_server().
 *
 * (1) Create socket based on type.
 * (2) If it is TCP (type is SOCK_STREAM) set socket option to reuse address.
 * (3) If it is UDP, add socket descriptor to set of open socket list.
 *     Socket descriptor for SOCK_STREAM will be added later after connection
 *     to the server already established (see send_tcp()).
 */
int Resolver::init(const int type)
{
	int s;

	if (! is_servers_set()) {
		fprintf(stderr, "[%s] init: no server set!\n", __cname);
		return -1;
	}

	FD_ZERO(&_fd_all);
	FD_ZERO(&_fd_read);

	// (1)
	s = create(PF_INET, type);
	if (s < 0) {
		return -1;
	}

	if (type == SOCK_STREAM) {
		// (2)
		s = set_reuse_address ();
		if (s < 0) {
			return -1;
		}
	} else {
		// (3)
		FD_SET(_d, &_fd_all);
		_maxfd = _d + 1;
	}

	return s;
}

//
// `servers_reset()` will clear all parent resolver addresses and buffer.
//
void Resolver::servers_reset()
{
	if (_servers) {
		_servers->reset();
	}
	Socket::reset();
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
int Resolver::set_server(const char* server_list)
{
	if (!server_list) {
		return 0;
	}
	if (is_servers_set()) {
		servers_reset();
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
int Resolver::add_server(const char* server_list)
{
	ListSockAddr *list_server;

	int s = ListSockAddr::NEW(&list_server, server_list, ',', PORT);
	if (s) {
		return -1;
	}

	Object *item = list_server->pop_head();
	while (item) {
		if (!_servers) {
			_servers = new ListSockAddr();
		}
		_servers->push_tail(item);
		item = list_server->pop_head();
	}

	delete list_server;

	return 0;
}

/**
 * @method		: Resolver::send_udp
 * @param		:
 *	> question	: pointer to DNS in UDP packet
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: send 'question' to one of DNS server.
 */
int Resolver::send_udp(DNSQuery* question)
{
	if (!question || _type != SOCK_DGRAM) {
		return -1;
	}
	if (! is_servers_set()) {
		fprintf(stderr, "[%s] send_udp: no server!\n", __cname);
		return -1;
	}

	SockAddr* sockaddr = _servers->rotate();

	int s;

	if (question->_bfr_type == BUFFER_IS_TCP) {
		s = question->to_udp();
		if (s < 0) {
			return -1;
		}
	}

	s = (int) Socket::send_udp(&sockaddr->_in, question);

	return s;
}

/**
 * @method		: Resolver::recv_udp
 * @param		:
 *	> answer	: output, pointer to DNS answer packet.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: receive DNS UDP packet from parent server.
 */
int Resolver::recv_udp(DNSQuery* answer)
{
	if (!answer || _type != SOCK_DGRAM) {
		return -1;
	}

	struct sockaddr_in addr;

	int s = (int) Socket::recv_udp(&addr);
	if (s <= 0) {
		return -1;
	}

	answer->reset(DNSQ_DO_ALL);
	answer->set((Buffer*) this);
	answer->extract (vos::DNSQ_EXTRACT_RR_AUTH);

	if (answer->_n_ans == 0 && answer->_n_aut == 0) {
		if (LIBVOS_DEBUG) {
			printf("[%s] recv_udp: number of RR answer '%d'\n"
				, __cname, answer->_n_ans);
		}
	}

	return 0;
}

/**
 * @method		: Resolver::send_tcp
 * @param		:
 *	> question	: pointer to DNS packet that will be send.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: send query through TCP channel.
 *
 * (1) Question MUST NOT be empty.
 * (2) Socket MUST HAVE created with SOCK_STREAM.
 * (3) Servers MUST HAVE already set.
 *
 * (4) If socket is closed,
 *	(4.1) create new socket descriptor,
 *	(4.2) and connect to the server.
 *	(4.3) If socket can't be opened because connection is already
 *	established (probably because server already close the connection),
 *	close the socket and try again.
 *	(4.4) Add socket descriptor for further select.
 *
 * (5) If question is UDP, convert it to TCP.
 */
int Resolver::send_tcp(DNSQuery* question)
{
	// (1),(2)
	if (!question || _type != SOCK_STREAM) {
		return -1;
	}
	// (3)
	if (! is_servers_set()) {
		fprintf(stderr, "[%s] send_tcp: no server!\n", __cname);
		return -1;
	}

	int s;
	SockAddr *sockaddr;

	// (4)
	if (_status < 0) {
		// (4.1)
		init (_type);

		sockaddr = _servers->rotate();

		_n_try = 0;
		do {
			// (4.2)
			s = connect_to(&sockaddr->_in);
			if (s < 0) {
				// (4.3)
				if (EISCONN == errno) {
					shutdown (_d, SHUT_RDWR);
				}
				sockaddr = _servers->rotate();
				_n_try++;
			} else {
				// (4.4)
				FD_SET(_d, &_fd_all);
				_maxfd = (_d > _maxfd ? _d : _maxfd);

				if (LIBVOS_DEBUG) {
					fprintf(stderr
					, "[%s] send_tcp: connected to server '%s'\n"
					, __cname, sockaddr->chars());
				}
			}
		} while (s != 0 && _n_try < N_TRY);

		if (s < 0) {
			if (LIBVOS_DEBUG) {
				fprintf(stderr
				, "[%s] send_tcp: cannot connect to server!\n"
				, __cname);
			}
			return -1;
		}

		FD_SET (_d, &_fd_all);
	}

	// (5)
	if (question->_bfr_type == BUFFER_IS_UDP) {
		s = question->to_tcp();
		if (s < 0) {
			return -1;
		}
	}

	Error err = write(question);
	if (err != NULL) {
		return -1;
	}

	if (LIBVOS_DEBUG) {
		fprintf(stdout
		, "[%s] send_tcp: %s\n", __cname, question->_name.v());

	}

	return 0;
}

/**
 * @method		: Resolver::recv_tcp
 * @param		:
 *	> answer	: output, pointer to DNS packet reply from parent
 *                        server.
 * @return		:
 * 	< 1		: success, one packet read.
 *	< 0		: no packet read.
 *	< -1		: fail.
 * @desc		: receive a reply from parent server through TCP
 * channel. DNS packet will be converted to UDP mode, without header length.
 *
 * (1) answer MUST NOT be empty.
 * (2) Socket MUST HAVE created with SOCK_STREAM.
 * (3) Socket is not closed.
 *
 * (4) Make sure is socket is ready for reading.
 * (5) Read DNS packet.
 * (6) If read return 0 (server closed the connection) remove socket
 * descriptor from set.
 * DO NOT close the socket, let upper layer handle closing connection.
 * (7) Convert packet to UDP and extract it.
 * (9) Check if number of records is greater than 0.
 */
int Resolver::recv_tcp(DNSQuery* answer)
{
	// (1),(2)
	if (!answer || _type != SOCK_STREAM) {
		return -1;
	}

	// (3)
	if (_status < 0) {
		if (LIBVOS_DEBUG) {
			fprintf (stderr
				, "[%s] recv_tcp: socket is not open.\n"
				, __cname);
		}
		return -1;
	}

	ssize_t s;

	_fd_read		= _fd_all;
	_timeout.tv_sec		= TIMEOUT;
	_timeout.tv_usec	= 0;

	// (4)
	s = select(_d + 1, &_fd_read, 0, 0, &_timeout);
	if (s < 0) {
		return -1;
	}

	if (! FD_ISSET(_d, &_fd_read)) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr
				, "[%s] recv_tcp: timeout after '%u' seconds.\n"
				, __cname, TIMEOUT);
		}
		return -1;
	}

	// (5)
	Error err = read();
	if (err != NULL) {
		reset();
		close();
		FD_CLR(_d, &_fd_all);

		// (6)
		if (err == ErrFileEnd) {
			if (LIBVOS_DEBUG) {
				printf ("[%s] recv_tcp: connection closed.\n"
					, __cname);
			}
			return 0;
		}
		return -1;
	}

	// (7)
	answer->reset(DNSQ_DO_ALL);
	answer->to_udp((Buffer *) this);
	answer->extract (vos::DNSQ_EXTRACT_RR_AUTH);

	// (9)
	if (answer->_n_ans == 0 && answer->_n_aut == 0) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr
				, "[%s] recv_tcp: number of RR answer '%d'\n"
				, __cname, answer->_n_ans);
		}
	}

	return 0;
}

/**
 * @method		: Resolver::resolve_udp
 * @param		:
 *	> question	: query to be send to server.
 *	> answer	: return value, answer from server.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	send 'question' to server to get an 'answer' using UDP protocol.
 */
int Resolver::resolve_udp(DNSQuery* question, DNSQuery* answer)
{
	if (!question || _type != SOCK_DGRAM) {
		return 0;
	}

	int s = 0;

	s = send_udp(question);
	if (s < 0) {
		return -1;
	}

	_n_try = 0;

	do {
		_fd_read		= _fd_all;
		_timeout.tv_sec		= TIMEOUT;
		_timeout.tv_usec	= 0;

		s = select(_maxfd, &_fd_read, NULL, NULL, &_timeout);
		if (s < 0) {
			if (EINTR == errno) {
				return -1;
			}
		}
		if (0 == s || 0 == FD_ISSET(_d, &_fd_read)) {
			++_n_try;
			if (LIBVOS_DEBUG) {
				printf(
"[%s] resolve_udp: timeout...(%u)\n", __cname, _n_try);
			}
			continue;
		}

		s = recv_udp(answer);
		if (s < 0) {
			continue;
		}

		s = question->_name.like(&answer->_name);
		if (s != 0) {
			if (LIBVOS_DEBUG) {
				printf(
			"[%s] resolve_udp: mismatch name [Q:%s] vs [A:%s]\n"
					, __cname
					, question->_name.chars()
					, answer->_name.chars());
			}
			continue;
		}

		if (question->_id != answer->_id) {
			if (LIBVOS_DEBUG) {
				printf(
			"[%s] resolve_udp: mismatch ID [Q:%d] vs [A:%d]\n"
					, __cname
					, question->_id, answer->_id);
			}
			answer->set_id(question->_id);
		}

		return 0;
	} while (_n_try < N_TRY);

	return -1;
}

/**
 * @method		: Resolver::resolve_tcp
 * @param		:
 *	> question	: query to be send to server.
 *	> answer	: return value, answer from server.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	send 'question' to server to get an 'answer' using TCP protocol.
 */
int Resolver::resolve_tcp(DNSQuery* question, DNSQuery* answer)
{
	if (!question || _type != SOCK_STREAM) {
		return -1;
	}

	int s = 0;

	s = send_tcp(question);
	if (s < 0) {
		return -1;
	}

	_n_try = 0;

	do {
		_fd_read		= _fd_all;
		_timeout.tv_sec		= TIMEOUT;
		_timeout.tv_usec	= 0;

		s = select(_maxfd, &_fd_read, NULL, NULL, &_timeout);
		if (s < 0) {
			if (EINTR == errno) {
				return -1;
			}
		}
		if (0 == s || !FD_ISSET(_d, &_fd_read)) {
			++_n_try;
			if (LIBVOS_DEBUG) {
				printf("[%s] resolve_tcp: timeout...(%u)\n"
					, __cname, _n_try);
			}
			continue;
		}

		s = recv_tcp(answer);
		if (s < 0) {
			break;
		}

		s = question->_name.like(&answer->_name);
		if (s != 0) {
			if (LIBVOS_DEBUG) {
				printf("[%s] resolve_tcp: mismatch name [Q:%s] vs [A:%s]\n"
					, __cname
					, question->_name.chars()
					, answer->_name.chars());
			}
			continue;
		}

		if (question->_id != answer->_id) {
			if (LIBVOS_DEBUG) {
				printf("[%s] resolve_tcp: mismatch ID [Q:%d] vs [A:%d]\n"
					, __cname
					, question->_id, answer->_id);
			}
			answer->set_id(question->_id);
		}
		return 0;
	} while (_n_try < N_TRY);

	return -1;
}

/**
 * @method		: Resolver::resolve
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
int Resolver::resolve(DNSQuery* question, DNSQuery* answer)
{
	if (!question || !answer) {
		return -1;
	}

	int s;

	if (_type == SOCK_STREAM) {
		s = resolve_tcp(question, answer);
	} else {
		s = resolve_udp(question, answer);
	}

	return s;
}

//
// `chars()` will return string representation of this object.
//
const char* Resolver::chars()
{
	if (__str) {
		free(__str);
		__str = NULL;
	}

	if (!_servers) {
		return NULL;
	}

	Buffer b;
	b.append_fmt("{ \"servers\": %s }", _servers->chars());

	__str = b.detach();

	return __str;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
