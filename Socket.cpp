/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Socket.hpp"

namespace vos {

int		Socket::DFLT_BUFFER_SIZE	= 65495;
unsigned int	Socket::DFLT_LISTEN_QUEUE	= 8;

Socket::Socket(int buffer_size) : File(buffer_size),
	_family(0),
	_port(0),
	_readfds(),
	_writefds(),
	_errorfds(),
	_timeout(),
	_clients(NULL),
	_next(NULL),
	_prev(NULL)
{
	_name.resize(255);
	FD_ZERO(&_readfds);
	FD_ZERO(&_writefds);
	FD_ZERO(&_errorfds);
}

Socket::~Socket()
{
	_next	= NULL;
	_prev	= NULL;
}

void Socket::create(const int family, const int type)
{
	_d = socket(family, type, 0);
	if (_d < 0)
		throw Error(E_SOCK_CREATE);

	_family	= family;
	_status = FILE_OPEN_NO;
}

void Socket::create_tcp()
{
	Socket::create();
}

void Socket::create_udp()
{
	Socket::create(AF_INET, SOCK_DGRAM);
}

void Socket::create_addr(struct sockaddr_in *sin, const char *address,
			const int port)
{
	int			s;
	int			buf_len	= 512;
	int			err	= 0;
	struct hostent		he;
	struct hostent		*hep	= NULL;
	char			*buf	= NULL;

	memset(sin, 0, sizeof(*sin));

	sin->sin_family	= _family;
	sin->sin_port	= htons(port);
	_port		= port;

	if (isdigit(address[0])) {
		s = inet_pton(_family, address, &sin->sin_addr);
		if (! s) {
			throw Error(E_SOCK_ADDR_INV, address);
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
			throw Error(E_SOCK_ADDR_RESOLV, address);
		}

		memcpy(&sin->sin_addr, hep->h_addr, hep->h_length);
		free(buf);
	}
}

void Socket::create_addr6(struct sockaddr_in6 *sin6, const char *address,
			const int port)
{
	int			s;
	int			buf_len	= 512;
	int			err	= 0;
	struct hostent		he;
	struct hostent		*hep	= NULL;
	char			*buf	= NULL;

	memset(sin6, 0, sizeof(*sin6));

	sin6->sin6_family	= _family;
	sin6->sin6_port		= htons(port);
	_port			= port;

	buf = strchr(address, ':');
	if (buf) {
		s = inet_pton(_family, address, &sin6->sin6_addr);
		if (! s)
			throw Error(E_SOCK_ADDR_INV, address);
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
			throw Error(E_SOCK_ADDR_RESOLV, address);
		}

		memcpy(&sin6->sin6_addr, hep->h_addr, hep->h_length);
		free(buf);
	}
}

void Socket::bind(const char *address, const int port)
{
	int s = 1;

	s = setsockopt(_d, SOL_SOCKET, SO_REUSEADDR, &s, sizeof(s));
	if (s) {
		throw Error(E_SOCK_ADDR_REUSE, address, port);
	}

	if (_family == AF_INET6) {
		struct sockaddr_in6 sin6;

		create_addr6(&sin6, address, port);
		s = ::bind(_d, (struct sockaddr *) &sin6,
			sizeof(sin6));
		if (s) {
			throw Error(E_SOCK_BIND, address, port);
		}
	} else {
		struct sockaddr_in sin;

		create_addr(&sin, address, port);
		s = ::bind(_d, (struct sockaddr *) &sin, sizeof(sin));
		if (s) {
			throw Error(E_SOCK_BIND, address, port);
		}
	}

	_status = FILE_OPEN_RW;
	_name.copy(address);
}

void Socket::listen(const unsigned int queue_len)
{
	::listen(_d, queue_len);
}

void Socket::bind_listen(const char *address, const int port)
{
	Socket::bind(address, port);
	::listen(_d, DFLT_LISTEN_QUEUE);
}

void Socket::connect_to(const char *address, const int port)
{
	int s = 0;

	if (_family == AF_INET6) {
		struct sockaddr_in6 sin6;

		create_addr6(&sin6, address, port);
		s = ::connect(_d, (struct sockaddr *) &sin6, sizeof(sin6));
	} else {
		struct sockaddr_in sin;

		create_addr(&sin, address, port);
		s = ::connect(_d, (struct sockaddr *) &sin, sizeof(sin));
	}

	if (s) {
		throw Error(E_SOCK_CONNECT, address, port);
	}

	_status = FILE_OPEN_RW;
	_name.copy(address);
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

Socket * Socket::accept()
{
	socklen_t		client_addrlen;
	struct sockaddr_in	client_addr	= {0};
	const char		*p		= NULL;
	Socket			*client		= new Socket();

	client_addrlen = sizeof(client_addr);
	client->_d = ::accept(_d, (struct sockaddr *) &client_addr,
				&client_addrlen);
	if (client->_d < 0) {
		throw Error(E_SOCK_ACCEPT);
	}

	do {
		p = inet_ntop(_family, &client_addr.sin_addr,
				client->_name._v, client->_name._l);
		if (! p)
			client->_name.resize(_name._l * 2);
	} while (NULL == p);

	client->_port	= ntohs(client_addr.sin_port);
	client->_status	= FILE_OPEN_RW;

	return client;
}

Socket * Socket::accept6()
{
	socklen_t		client_addrlen;
	struct sockaddr_in6	client_addr;
	const char		*p	= NULL;
	Socket			*client = new Socket();

	client_addrlen = sizeof(client_addr);
	client->_d = ::accept(_d, (struct sockaddr *) &client_addr,
				&client_addrlen);
	if (client->_d < 0) {
		throw Error(E_SOCK_ACCEPT);
	}

	do {
		p = inet_ntop(_family, &client_addr.sin6_addr,
				client->_name._v, client->_name._l);
		if (! p)
			_name.resize(_name._l * 2);
	} while (NULL == p);

	client->_port	= ntohs(client_addr.sin6_port);
	client->_status	= FILE_OPEN_RW;

	return client;
}

Socket * Socket::accept_conn()
{
	Socket *client = NULL;

	if (_family == AF_INET6) {
		client = accept();
	} else {
		client = accept6();
	}
	
	printf(" accepting connection from '%s'\n", client->_name._v);

	add_client(client);

	return client;
}

void Socket::send(Buffer *bfr)
{
	if (! bfr)
		return;

	File::write(bfr->_v, bfr->_i);
	File::flush();
}

int Socket::send_udp(struct sockaddr *addr, Buffer *bfr)
{
	int n_send;

	n_send = ::sendto(_d, bfr->_v, bfr->_i, 0, addr,
				sizeof(struct sockaddr));

	return n_send;
}

int Socket::recv_udp(struct sockaddr *addr)
{
	socklen_t addr_len = sizeof(struct sockaddr);

	_i = ::recvfrom(_d, _v, _l, 0, addr, &addr_len);

	printf(">> UDP receive : %d bytes\n", _i);

	return _i;
}

} /* namespace::vos */
