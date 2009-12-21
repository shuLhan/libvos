/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Socket.hpp"

namespace vos {

unsigned int	Socket::DFLT_BUFFER_SIZE	= 65495;
unsigned int	Socket::DFLT_LISTEN_SIZE	= 4;
unsigned int	Socket::DFLT_NAME_SIZE		= 255;
const char*	Socket::ADDR_WILCARD		= "0.0.0.0";

Socket::Socket() : File(),
	_family(0),
	_port(0),
	_timeout(),
	_clients(NULL),
	_next(NULL),
	_prev(NULL)
{}

Socket::~Socket()
{
	_next	= NULL;
	_prev	= NULL;
	if (_clients) {
		delete _clients;
		_clients = NULL;
	}
}

int Socket::init(const int bfr_size)
{
	int s;

	s = File::init(bfr_size);
	if (s < 0)
		return s;

	s = _name.resize(DFLT_NAME_SIZE);
	if (s < 0)
		return s;

	return 0;
}

/**
 * @desc		: create a new socket descriptor.
 *
 * @param		:
 *	> family	: address family (AF_LOCAL, AF_UNIX, AF_INET, etc).
 *	> type		: socket type (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW).
 *
 * @return:
 *	< 0		: success.
 *	< <0		: fail.
 */
int Socket::create(const int family, const int type)
{
	int s = 0;

	if (!_v) {
		s = Socket::init(DFLT_BUFFER_SIZE);
		if (s < 0)
			return s;
	}

	_d = ::socket(family, type, 0);
	if (_d < 0)
		return -E_SOCK_CREATE;

	_family	= family;
	_status = vos::FILE_OPEN_NO;

	return 0;
}

int Socket::create_tcp()
{
	return Socket::create(PF_INET, SOCK_STREAM);
}

int Socket::create_udp()
{
	return Socket::create(PF_INET, SOCK_DGRAM);
}

/**
 * @desc	: create socket address from 'address' and 'port'.
 *
 * @param		:
 *	< sin		: return value, socket address object.
 *	< address	: hostname or IP address.
 *	< port		: port number.
 *
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 */
int Socket::create_addr(struct sockaddr_in *sin, const char *address,
			const int port)
{
	int		s;
	int		buf_len	= 512;
	int		err	= 0;
	struct hostent	he;
	struct hostent	*hep	= NULL;
	char		*buf	= NULL;

	memset(sin, 0, sizeof(struct sockaddr_in));

	sin->sin_family		= _family;
	sin->sin_port		= htons(port);
	_port			= port;

	if (isdigit(address[0])) {
		s = inet_pton(_family, address, &sin->sin_addr);
		if (s <= 0) {
			return -E_SOCK_ADDR_INV;
		}
	} else {
		do {
			buf = (char *) calloc(buf_len, sizeof(char));
			s = gethostbyname2_r(address, _family, &he, buf,
						buf_len, &hep, &err);
			if (ERANGE == s) {
				free(buf);
				buf_len *= 2;
			}
		} while (ERANGE == s);

		if (err) {
			free(buf);
			return -E_SOCK_ADDR_RESOLV;
		}

		memcpy(&sin->sin_addr, hep->h_addr, hep->h_length);
		free(buf);
	}
	return 0;
}

/**
 * @desc		: create socket address from 'address' and 'port'.
 *
 * @param		:
 *	< sin6		: return value, socket address object.
 *	< address	: hostname or IP address.
 *	< port		: port number.
 *
 * @return		:
 *	< 0		: success.
 *	< !0		: fail.
 */
int Socket::create_addr6(struct sockaddr_in6 *sin6, const char *address,
				const int port)
{
	int		s;
	int		buf_len	= 512;
	int		err	= 0;
	struct hostent	he;
	struct hostent	*hep	= NULL;
	char		*buf	= NULL;

	memset(sin6, 0, sizeof(*sin6));

	sin6->sin6_family	= _family;
	sin6->sin6_port		= htons(port);
	_port			= port;

	buf = (char *) strchr(address, ':');
	if (buf) {
		s = inet_pton(_family, address, &sin6->sin6_addr);
		if (!s) {
			return -E_SOCK_ADDR_INV;
		}
	} else {
		do {
			buf = (char *) calloc(buf_len, sizeof(char));
			s = gethostbyname2_r(address, _family, &he, buf,
						buf_len, &hep, &err);
			if (ERANGE == s) {
				free(buf);
				buf_len *= 2;
			}
		} while (ERANGE == s);

		if (err) {
			free(buf);
			return -E_SOCK_ADDR_RESOLV;
		}

		memcpy(&sin6->sin6_addr, hep->h_addr, hep->h_length);
		free(buf);
	}
	return 0;
}

/**
 * @desc		: bind socket to 'address' and 'port'.
 *
 * @param		:
 *	> address	: hostname or IP address to bind to.
 *	> port		: port number.
 *
 * @return			:
 *	< 0			: success.
 *	< -E_SOCK_ADDR_INV	: fail, address invalid.
 *	< -E_SOCK_ADDR_REUSE	: fail, address and port is already used.
 *	< -E_SOCK_BIND		: fail, cannot bind to address:port.
 *	< <0			: fail.
 */
int Socket::bind(const char *address, const int port)
{
	int	s	= 0;
	int	optval	= 1;

	if (LIBVOS_DEBUG) {
		printf("[VOS.SOCKET] bind: %s:%d\n", address, port);
	}

	if (!address) {
		address = ADDR_WILCARD;
	} else {
		s = strlen(address);
		if (0 == s) {
			address = ADDR_WILCARD;
		}
	}

	s = setsockopt(_d, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
	if (s != 0) {
		return -E_SOCK_ADDR_REUSE;
	}

	if (_family == AF_INET6) {
		struct sockaddr_in6 sin6;

		s = create_addr6(&sin6, address, port);
		if (s < 0)
			return s;

		s = ::bind(_d, (struct sockaddr *) &sin6, sizeof(sin6));
	} else {
		struct sockaddr_in sin;

		s = create_addr(&sin, address, port);
		if (s < 0)
			return s;

		s = ::bind(_d, (struct sockaddr *) &sin, sizeof(sin));
	}
	if (s < 0) {
		return -E_SOCK_BIND;
	}

	_status = vos::FILE_OPEN_RW;

	s = _name.copy_raw(address, 0);

	return s;
}

/**
 * @return			:
 *	< 0			: success.
 *	< -E_SOCK_LISTEN	: fail.
 */
int Socket::listen(const unsigned int queue_len)
{
	int s;

	s = ::listen(_d, queue_len);
	if (s < 0)
		return -E_SOCK_LISTEN;

	return 0;
}

/**
 * @return			:
 *	< 0			: success.
 *	< -E_SOCK_LISTEN	: fail.
 */
int Socket::bind_listen(const char *address, const int port)
{
	int s;

	s = Socket::bind(address, port);
	if (0 == s) {
		s = ::listen(_d, 0);
		if (s != 0) {
			s = -E_SOCK_LISTEN;
		}
	}
	return s;
}

/**
 * @desc		: connect socket to 'address' with 'port'.
 *
 * @param		:
 *	> address	: destination hostname or IP address to connect to.
 *	> port		: destination port number.
 *
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 */
int Socket::connect_to(const char *address, const int port)
{
	int s;

	if (_family == AF_INET6) {
		struct sockaddr_in6 sin6;

		s = create_addr6(&sin6, address, port);
		if (s < 0)
			return s;

		s = ::connect(_d, (struct sockaddr *) &sin6, sizeof(sin6));
	} else {
		struct sockaddr_in sin;

		s = create_addr(&sin, address, port);
		if (s < 0)
			return s;

		s = ::connect(_d, (struct sockaddr *) &sin, sizeof(sin));
	}
	if (s < 0) {
		return -E_SOCK_CONNECT;
	}

	_status	= vos::FILE_OPEN_RW;
	s	= _name.copy_raw(address, 0);

	return s;
}

void Socket::add_client(Socket *client)
{
	Socket *p = _clients;

	if (! p) {
		_clients = client;
	} else {
		while (p->_next)
			p = p->_next;
		p->_next	= client;
		client->_prev	= p;
	}
}

void Socket::remove_client(Socket *client)
{
	if (! client)
		return;

	if (! client->_prev) {
		_clients = _clients->_next;
	} else {
		client->_prev->_next = client->_next;
	}

	if (client->_next) {
		client->_next->_prev = client->_prev;
	}

	delete client;
}

/**
 * @return		:
 *	< Socket*	: success, new client accepted.
 *	< NULL		: fail.
 */
Socket * Socket::accept()
{
	int			s;
	socklen_t		client_addrlen;
	struct sockaddr_in	client_addr;
	const char		*p		= NULL;
	Socket			*client		= NULL;

	client = new Socket();
	if (!client)
		return NULL;

	s = client->init(Socket::DFLT_BUFFER_SIZE);
	if (s != 0) {
		delete client;
		return NULL;
	}

	client_addrlen = sizeof(client_addr);
	client->_d = ::accept(_d, (struct sockaddr *) &client_addr,
				&client_addrlen);
	if (client->_d < 0) {
		delete client;
		return NULL;
	}

	do {
		p = inet_ntop(_family, &client_addr.sin_addr,
				client->_name._v, client->_name._l);
		if (! p)
			client->_name.resize(_name._l * 2);
	} while (NULL == p);

	client->_port	= ntohs(client_addr.sin_port);
	client->_status	= vos::FILE_OPEN_RW;

	return client;
}

Socket * Socket::accept6()
{
	int			s;
	socklen_t		client_addrlen;
	struct sockaddr_in6	client_addr;
	const char		*p	= NULL;
	Socket			*client = NULL;

	client = new Socket();
	if (!client)
		return NULL;

	s = client->init(Socket::DFLT_BUFFER_SIZE);
	if (s != 0) {
		delete client;
		return NULL;
	}

	client_addrlen = sizeof(client_addr);
	client->_d = ::accept(_d, (struct sockaddr *) &client_addr,
				&client_addrlen);
	if (client->_d < 0) {
		delete client;
		return NULL;
	}

	do {
		p = inet_ntop(_family, &client_addr.sin6_addr,
				client->_name._v, client->_name._l);
		if (! p)
			_name.resize(_name._l * 2);
	} while (NULL == p);

	client->_port	= ntohs(client_addr.sin6_port);
	client->_status	= vos::FILE_OPEN_RW;

	return client;
}

Socket * Socket::accept_conn()
{
	Socket *client = NULL;

	if (_family == AF_INET6) {
		client = accept6();
	} else {
		client = accept();
	}
	if (!client)
		return NULL;

	if (LIBVOS_DEBUG) {
		printf("[VOS.SOCKET] accepting connection from '%s'\n",
			client->_name._v);
	}

	add_client(client);

	return client;
}

int Socket::send(Buffer *bfr)
{
	int s;

	if (!bfr) {
		if (LIBVOS_DEBUG) {
			dump_hex();
		}
	} else {
		if (LIBVOS_DEBUG) {
			bfr->dump_hex();
		}

		s = write_raw(bfr->_v, bfr->_i);
		if (s < 0)
			return s;
	}

	s = flush();

	return s;
}

int Socket::send_raw(const char *bfr, const int len)
{
	int s;

	if (bfr) {
		s = write_raw(bfr, len);
		if (s < 0)
			return s;
	}
	s = flush();
	return s;
}

int Socket::send_udp(struct sockaddr *addr, Buffer *bfr)
{
	int n_send;

	n_send = ::sendto(_d, bfr->_v, bfr->_i, 0, addr,
				sizeof(struct sockaddr));

	return n_send;
}

int Socket::send_udp_raw(struct sockaddr *addr, const char *bfr,
				const int len)
{
	int n_send;

	if (!addr)
		return 0;
	if (!bfr)
		return 0;
	if (!len)
		return 0;

	n_send = ::sendto(_d, bfr, len, 0, addr, sizeof(struct sockaddr));

	return n_send;
}

int Socket::recv_udp(struct sockaddr *addr)
{
	socklen_t addr_len = sizeof(struct sockaddr);

	_i = ::recvfrom(_d, _v, _l, 0, addr, &addr_len);

	return _i;
}

} /* namespace::vos */
