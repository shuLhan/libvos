/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#include "Resolver.hh"

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
,	_maxfd(0)
,	_fd_all()
,	_fd_read()
,	_n_try(0)
,	_timeout()
,	_servers(NULL)
,	_p_server(NULL)
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

	if (!_servers) {
		fprintf(stderr, "[vos::Resolver] init: no server set!\n");
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
	int		port_num;
	Buffer		addr;
	Buffer		port;
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
		if (*p == ':') {
			p++;
			while (isalnum(*p)) {
				port.appendc(*p);
				p++;
			}
		}
		while (isspace(*p)) {
			p++;
		}
		if (!*p) {
			break;
		}
		if (*p != ',') {
			fprintf(stderr
			, "[Resolver::add_server] invalid character: '%c'\n"
			, *p);
			return -1;
		}
		p++;
		if (!addr.is_empty()) {
			port_num = (int) port.to_lint ();
			if (port_num <= 0 || port_num > 65534) {
				port_num = PORT;
			}
			s = SockAddr::INIT(&saddr, AF_INET, addr._v, port_num);
			if (s < 0) {
				return -1;
			}
			SockAddr::ADD(&_servers, saddr);
			addr.reset();
		}
	}
	if (!addr.is_empty()) {
		port_num = (int) port.to_lint ();
		if (port_num <= 0 || port_num > 65534) {
			port_num = PORT;
		}
		s = SockAddr::INIT(&saddr, AF_INET, addr._v, port_num);
		if (s < 0) {
			return -1;
		}
		SockAddr::ADD(&_servers, saddr);
	}

	return 0;
}

/**
 * @method	: Resolver::rotate_server
 * @desc	: switch or change parent server to another one in the list.
 */
void Resolver::rotate_server()
{
	if (!_p_server) {
		_p_server = _servers;
	} else {
		_p_server = _p_server->_next;
		if (!_p_server) {
			_p_server = _servers;
		}
	}
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
	if (!_servers) {
		fprintf(stderr, "[vos::Resolver] send_udp: no server!\n");
		return -1;
	}

	rotate_server();

	if (LIBVOS_DEBUG) {
		printf("[vos::Resolver] send_udp: server '%s' ...\n"
			, _p_server->chars());
	}

	int s;

	if (question->_bfr_type == BUFFER_IS_TCP) {
		s = question->to_udp();
		if (s < 0) {
			return -1;
		}
	}

	s = (int) Socket::send_udp(&_p_server->_in, question);

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

	register int		s;
	struct sockaddr_in	addr;

	s = (int) Socket::recv_udp(&addr);
	if (s <= 0) {
		return s;
	}

	answer->reset(DNSQ_DO_ALL);
	answer->set((Buffer*) this);
	answer->extract (vos::DNSQ_EXTRACT_RR_AUTH);

	s = 0;

	if ((answer->_flag & RCODE_FLAG) != 0) {
		if (LIBVOS_DEBUG) {
			printf("[vos::Resolver] recv_udp: reply flag is zero.\n");
		}
		s = -1;
	} else if (answer->_n_ans <= 0 && answer->_n_aut <= 0) {
		if (LIBVOS_DEBUG) {
			printf("[vos::Resolver] recv_udp: number of RR answer '%d'\n"
				, answer->_n_ans);
		}
		s = -1;
	}

	return 1;
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
	if (!_servers) {
		fprintf(stderr, "[vos::Resolver] send_tcp: no server!\n");
		return -1;
	}

	int s;

	// (4)
	if (_status < 0) {
		// (4.1)
		init (_type);

		if (!_p_server) {
			rotate_server();
		}

		_n_try = 0;
		do {
			// (4.2)
			s = connect_to(&_p_server->_in);
			if (s < 0) {
				// (4.3)
				if (EISCONN == errno) {
					shutdown (_d, SHUT_RDWR);
				}
				rotate_server();
				_n_try++;
			} else {
				// (4.4)
				FD_SET(_d, &_fd_all);
				_maxfd = (_d > _maxfd ? _d : _maxfd);

				if (LIBVOS_DEBUG) {
					fprintf(stderr
					, "[vos::Resolver] send_tcp: connected to server '%s'\n"
					, _p_server->chars());
				}
			}
		} while (s != 0 && _n_try < N_TRY);

		if (s < 0) {
			if (LIBVOS_DEBUG) {
				fprintf(stderr
				, "[vos::Resolver] send_tcp: cannot connect to server!\n");
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

	s = (int) write(question);
	if (s < 0) {
		return -1;
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
 * (8) Check if reply header has REPLY flag.
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
				, "[vos::Resolver] recv_tcp: socket is not open.\n");
		}
		return -1;
	}

	int s;

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
				, "[vos::Resolver] recv_tcp: timeout after '%d' seconds.\n"
				, TIMEOUT);
		}
		return -1;
	}

	// (5)
	s = read();
	if (s < 0) {
		return -1;
	}

	// (6)
	if (s == 0) {
		if (LIBVOS_DEBUG) {
			printf ("[vos::Resolver] recv_tcp: connection closed.\n");
		}
		FD_CLR (_d, &_fd_all);
		return 0;
	}

	// (7)
	answer->reset(DNSQ_DO_ALL);
	answer->to_udp((Buffer *) this);
	answer->extract (vos::DNSQ_EXTRACT_RR_AUTH);

	// (8)
	if ((answer->_flag & RCODE_FLAG) != 0) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr
				, "[vos::Resolver] recv_tcp: reply flag is zero.\n");
		}
		return -1;
	// (9)
	} else if (answer->_n_ans <= 0 && answer->_n_aut <= 0) {
		if (LIBVOS_DEBUG) {
			fprintf(stderr
				, "[vos::Resolver] recv_tcp: number of RR answer '%d'\n"
				, answer->_n_ans);
		}
		return -1;
	}

	return 1;
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
"[vos::Resolver] resolve_udp: timeout...(%d)\n", _n_try);
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
"[vos::Resolver] resolve_udp: mismatch name [Q:%s] vs [A:%s]\n"
					, question->_name.chars()
					, answer->_name.chars());
			}
			continue;
		}

		if (question->_id != answer->_id) {
			if (LIBVOS_DEBUG) {
				printf(
"[vos::Resolver] resolve_udp: mismatch ID [Q:%d] vs [A:%d]\n"
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
				printf(
"[vos::Resolver] resolve_tcp: timeout...(%d)\n", _n_try);
			}
			continue;
		}

		s = recv_tcp(answer);
		if (s < 0) {
			continue;
		}

		s = question->_name.like(&answer->_name);
		if (s != 0) {
			if (LIBVOS_DEBUG) {
				printf(
"[vos::Resolver] resolve_tcp: mismatch name [Q:%s] vs [A:%s]\n"
					, question->_name.chars()
					, answer->_name.chars());
			}
			continue;
		}

		if (question->_id != answer->_id) {
			if (LIBVOS_DEBUG) {
				printf(
"[vos::Resolver] resolve_tcp: mismatch ID [Q:%d] vs [A:%d]\n"
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

	register int s;

	if (_type == SOCK_STREAM) {
		s = resolve_tcp(question, answer);
	} else {
		s = resolve_udp(question, answer);
	}

	return s;
}

} /* namespace::vos */
