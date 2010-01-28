/*
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
	_client_lock(),
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
	pthread_mutex_destroy(&_client_lock);
}

void Socket::lock_client()
{
	while (pthread_mutex_trylock(&_client_lock) != 0)
		;
}

void Socket::unlock_client()
{
	while (pthread_mutex_unlock(&_client_lock) != 0)
		;
}

/**
 * @method		: Socket::init
 * @param		:
 *	> bfr_size	: initial size for Socket buffer.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: initialize Socket object.
 */
int Socket::init(const int bfr_size)
{
	register int s;

	s = pthread_mutex_init(&_client_lock, NULL);
	if (s)
		return s;

	s = Buffer::init_size(bfr_size);
	if (s < 0)
		return s;

	s = _name.resize(DFLT_NAME_SIZE);
	if (s < 0)
		return s;

	return 0;
}

/**
 * @method		: Socket::create
 * @param		:
 *	> family	: address family (AF_LOCAL, AF_UNIX, AF_INET, etc).
 *	> type		: socket type (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW).
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: create a new socket descriptor.
 */
int Socket::create(const int family, const int type)
{
	register int s;

	if (!_v) {
		s = Socket::init();
		if (s < 0)
			return s;
	}

	_d = ::socket(family, type, 0);
	if (_d < 0)
		return -1;

	_family	= family;
	_status = vos::FILE_OPEN_NO;

	return 0;
}

/**
 * @method	: Socket::create_tcp
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: create stream socket for TCP connection.
 */
int Socket::create_tcp()
{
	return Socket::create(PF_INET, SOCK_STREAM);
}

/**
 * @method	: Socket::create_udp
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: create datagram socket for UDP connection.
 */
int Socket::create_udp()
{
	return Socket::create(PF_INET, SOCK_DGRAM);
}


/**
 * @method			: Socket::bind
 * @param			:
 *	> address		: hostname or IP address to bind to.
 *	> port			: port number.
 * @return			:
 *	< 0			: success.
 *	< -E_SOCK_ADDR_INV	: fail, address invalid.
 *	< -E_SOCK_ADDR_REUSE	: fail, address and port is already used.
 *	< -E_SOCK_BIND		: fail, cannot bind to address:port.
 *	< <0			: fail.
 * @desc			: bind socket to 'address' and 'port'.
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

		s = CREATE_ADDR6(&sin6, address, port);
		if (s < 0)
			return s;

		s = ::bind(_d, (struct sockaddr *) &sin6, sizeof(sin6));
	} else {
		struct sockaddr_in sin;

		s = CREATE_ADDR(&sin, address, port);
		if (s < 0)
			return s;

		s = ::bind(_d, (struct sockaddr *) &sin, sizeof(sin));
	}
	if (s < 0) {
		return -E_SOCK_BIND;
	}

	_status = O_RDWR;
	s	= _name.copy_raw(address, 0);
	_port	= port;

	return s;
}

/**
 * @method			: Socket::listen
 * @param			:
 *	> queue_len		: length of queue for client connections.
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
 * @method			: Socket::bind_listen
 * @param			:
 *	> address		: IP address or host name to listen to.
 *	> port			: port for accepting client connection.
 * @return			:
 *	< 0			: success.
 *	< -E_SOCK_LISTEN	: fail.
 * @desc			: create a socket that accepting client
 *                                connection at 'address' and 'port'.
 */
int Socket::bind_listen(const char *address, const int port)
{
	register int s;

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
 * @method		: Socket::connect_to
 * @param		:
 *	> address	: destination hostname or IP address to connect to.
 *	> port		: destination port number.
 * @return		:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: connect socket to 'address' with 'port'.
 */
int Socket::connect_to(const char *address, const int port)
{
	register int s;

	if (_family == AF_INET6) {
		struct sockaddr_in6 sin6;

		s = CREATE_ADDR6(&sin6, address, port);
		if (s < 0)
			return s;

		s = ::connect(_d, (struct sockaddr *) &sin6, sizeof(sin6));
	} else {
		struct sockaddr_in sin;

		s = CREATE_ADDR(&sin, address, port);
		if (s < 0)
			return s;

		s = ::connect(_d, (struct sockaddr *) &sin, sizeof(sin));
	}
	if (s < 0) {
		return -E_SOCK_CONNECT;
	}

	_status	= O_RDWR;
	s	= _name.copy_raw(address, 0);
	_port	= port;

	return s;
}

/**
 * @method		: Socket::add_client_r
 * @param		:
 *	> client	: socket object.
 * @desc		: add client object to list of clients.
 */
void Socket::add_client_r(Socket *client)
{
	lock_client();
	_clients = ADD_CLIENT(_clients, client);
	unlock_client();
}

/**
 * @method		: Socket::remove_client
 * @param		:
 *	> client	: Socket object.
 * @desc		:
 *	remove client object pointed by 'client' from list of clients.
 */
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

	client->_next = NULL;
	client->_prev = NULL;
}

/**
 * @method		: Socket::remove_client
 * @param		:
 *	> client	: Socket object.
 * @desc		:
 *	remove client object pointed by 'client' from list of clients.
 *	this is a reentrant version of remove_client().
 */
void Socket::remove_client_r(Socket *client)
{
	lock_client();
	remove_client(client);
	unlock_client();
}

/**
 * @method		: Socket::accept
 * @return		:
 *	< Socket*	: success, new client accepted.
 *	< NULL		: fail.
 * @desc		:
 *	This method is used by server socket for accepting a new client
 *	connection.
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

	s = client->init();
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
	client->_status	= O_RDWR;

	return client;
}

/**
 * @method		: Socket::accept6
 *	< Socket*	: success, new client accepted.
 *	< NULL		: fail.
 * @desc		:
 *	This method is used by server socket for accepting a new client
 *	connection. This is a for IPv6 server socket.
 */
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

	s = client->init();
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
	client->_status	= O_RDWR;

	return client;
}

/**
 * @method		: Socket::accept_conn
 *	< Socket*	: success, new client accepted.
 *	< NULL		: fail.
 * @desc		:
 *	This method is used by server socket for accepting a new client
 *	connection. This is a generic version.
 */
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

	add_client_r(client);

	return client;
}

/**
 * @method	: Socket::send
 * @param	:
 *	> bfr	: Buffer object, data that will be send.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: send data 'bfr' to end point connection.
 */
int Socket::send(Buffer *bfr)
{
	register int s;

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

/**
 * @method	: Socket::send_raw
 * @param	:
 *	> bfr	: raw data.
 *	> len	: length of 'bfr'.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: send data 'bfr' with length 'len' to end point connection.
 */
int Socket::send_raw(const char *bfr, const int len)
{
	register int s;

	if (bfr) {
		s = write_raw(bfr, len);
		if (s < 0)
			return s;
	}
	s = flush();
	return s;
}

/**
 * @method	: Socket::send_udp
 * @param	:
 *	> addr	: address of end point.
 *	> bfr	: buffer object to be send.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: send 'bfr' to 'addr' using datagram protocol.
 */
int Socket::send_udp(struct sockaddr *addr, Buffer *bfr)
{
	register int n_send;

	n_send = ::sendto(_d, bfr->_v, bfr->_i, 0, addr,
				sizeof(struct sockaddr));

	return n_send;
}

/**
 * @method	: Socket::send_udp_raw
 * @param	:
 *	> addr	: address of end point.
 *	> bfr	: buffer object to be send.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	:
 *	send 'bfr' with length is 'len' to 'addr' using datagram protocol.
 */
int Socket::send_udp_raw(struct sockaddr *addr, const char *bfr,
				const int len)
{
	register int n_send;

	if (!addr)
		return 0;
	if (!bfr)
		return 0;
	if (!len)
		return 0;

	n_send = ::sendto(_d, bfr, len, 0, addr, sizeof(struct sockaddr));

	return n_send;
}

/**
 * @method	: Socket::recv_udp
 * @param	:
 *	> addr	: originating address of end point.
 * @return	:
 *	< >=0	: success, size of received data.
 *	< <0	: fail.
 * @desc	:
 *	received data from end point using datagram protocl and save the end
 *	point address to 'addr'.
 */
int Socket::recv_udp(struct sockaddr *addr)
{
	socklen_t addr_len = sizeof(struct sockaddr);

	_i = ::recvfrom(_d, _v, _l, 0, addr, &addr_len);
	_v[_i] = '\0';

	return _i;
}

/**
 * @method	: Socket::IS_IPV4
 * @param	:
 *	> str	: string to check for.
 * @return	:
 *	< 1	: true.
 *	< 0	: false.
 * @desc	: check if 'str' is IPv4 address and it is valid address.
 *	Minimum length of IPv4 address is x.x.x.x == 7, and
 *	maximum length of IPv4 address is xxx.xxx.xxx.xxx == 15.
 *
 */
int Socket::IS_IPV4(const char *str)
{
	if (!str) {
		return 0;
	}
	register int	n	= 0;
	register int	dot	= 0;
	char		x[3];

	while (*str) {
		if (isdigit(*str)) {
			x[n] = *str;
			n++;
		} else if (*str == '.') {
			if (n == 0 || n > 4) {
				return 0;
			}
			if (n == 3) {
				if (x[0] > '2') {
					return 0;
				}
				if (x[1] > '5') {
					return 0;
				}
				if (x[2] > '5') {
					return 0;
				}
			}
			n = 0;
			dot++;
		} else {
			return 0;
		}
		*str++;
	}
	if (dot != 3 || n == 0 || n > 4)  {
		return 0;
	}
	if (n == 3) {
		if (x[0] > '2') {
			return 0;
		}
		if (x[1] > '5') {
			return 0;
		}
		if (x[2] > '5') {
			return 0;
		}
	}

	return 1;
}

/**
 * @method	: Socket::CREATE_ADDR
 * @param	:
 *	> sin	: return value, struct sockaddr_in object.
 *	> addr	: hostname or IPv4 address.
 *	> port	: port number.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	:
 *	create a IPv4 socket address object from address 'addr' and port
 *	'port', with address family default to internet (AF_INET).
 */
int Socket::CREATE_ADDR(struct sockaddr_in *sin, const char *addr,
			const int port)
{
	int		s;
	int		buf_len	= 512;
	int		err	= 0;
	struct hostent	he;
	struct hostent	*hep	= NULL;
	char		*buf	= NULL;

	memset(sin, 0, sizeof(struct sockaddr_in));

	sin->sin_family	= AF_INET;
	sin->sin_port	= htons(port);

	if (IS_IPV4(addr)) {
		s = inet_pton(AF_INET, addr, &sin->sin_addr);
		if (s <= 0) {
			return -1;
		}
	} else {
		do {
			buf = (char *) calloc(buf_len, sizeof(char));
			s = gethostbyname2_r(addr, AF_INET, &he, buf,
						buf_len, &hep, &err);
			if (ERANGE == s) {
				free(buf);
				buf_len *= 2;
			}
		} while (ERANGE == s);

		if (err) {
			free(buf);
			return -1;
		}

		memcpy(&sin->sin_addr, hep->h_addr, hep->h_length);
		free(buf);
	}
	return 0;
}

/**
 * @method	: Socket::CREATE_ADDR6
 * @param	:
 *	< sin6	: return value, struct sockaddr_in6 object.
 *	< addr	: hostname or IP address.
 *	< port	: port number.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	:
 *	create IPv6 internet address using 'address' and 'port'.
 */
int Socket::CREATE_ADDR6(struct sockaddr_in6 *sin6, const char *addr,
				const int port)
{
	int		s;
	int		buf_len	= 512;
	int		err	= 0;
	struct hostent	he;
	struct hostent	*hep	= NULL;
	char		*buf	= NULL;

	memset(sin6, 0, sizeof(struct sockaddr_in6));

	sin6->sin6_family	= AF_INET6;
	sin6->sin6_port		= htons(port);

	buf = (char *) strchr(addr, ':');
	if (buf) {
		s = inet_pton(AF_INET6, addr, &sin6->sin6_addr);
		if (s <= 0) {
			return -1;
		}
	} else {
		do {
			buf = (char *) calloc(buf_len, sizeof(char));
			s = gethostbyname2_r(addr, AF_INET6, &he, buf,
						buf_len, &hep, &err);
			if (ERANGE == s) {
				free(buf);
				buf_len *= 2;
			}
		} while (ERANGE == s);

		if (err) {
			free(buf);
			return -1;
		}

		memcpy(&sin6->sin6_addr, hep->h_addr, hep->h_length);
		free(buf);
	}
	return 0;
}

/**
 * @method		: Socket::ADD_CLIENT
 * @param		:
 *	> list		: Socket object, as the head of the list.
 *	> client	: a new client that will add to list of clients.
 * @return		:
 *	< Socket*	: pointer to a new head of list.
 * @desc		: add 'client' to the list 'list'.
 */
Socket * Socket::ADD_CLIENT(Socket *list, Socket *client)
{
	if (!list)
		return client;

	Socket *p = list;

	while (p->_next)
		p = p->_next;

	p->_next = client;

	return list;
}

} /* namespace::vos */
