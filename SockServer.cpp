/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#include "SockServer.hpp"

namespace vos {

const char* SockServer::ADDR_WILCARD	= "0.0.0.0";
const char* SockServer::ADDR_WILCARD6	= "::";

SockServer::SockServer() : Socket()
,	_timeout()
,	_client_lock()
,	_clients(NULL)
{
	pthread_mutex_init(&_client_lock, NULL);
}

SockServer::~SockServer()
{	
	Socket* next = NULL;

	while (_clients) {
		next = _clients->_next;
		delete _clients;
		_clients = next;
	}

	_next = NULL;
	_prev = NULL;
	pthread_mutex_destroy(&_client_lock);
}

/**
 * @method	: SockServer::lock_client
 * @desc	: block other thread from accessing list of client objects.
 */
void SockServer::lock_client()
{
	while (pthread_mutex_trylock(&_client_lock) != 0)
		;
}

/**
 * @method	: SockServer::unlock_client
 * @desc	: allow other thread to access list of client objects.
 */
void SockServer::unlock_client()
{
	while (pthread_mutex_unlock(&_client_lock) != 0)
		;
}

/**
 * @method			: SockServer::bind
 * @param			:
 *	> address		: hostname or IP address to bind to.
 *	> port			: port number.
 * @return			:
 *	< 0			: success.
 *	< -1			: fail.
 * @desc			: bind socket to 'address' and 'port'.
 */
int SockServer::bind(const char* address, const int port)
{
	int	s	= 0;
	int	optval	= 1;

	if (LIBVOS_DEBUG) {
		printf("[vos::SockSrvr] bind: %s:%d\n", address, port);
	}

	if (!address) {
		if (_family == AF_INET) {
			address = ADDR_WILCARD;
		} else {
			address = ADDR_WILCARD6;
		}
	} else {
		s = (int) strlen(address);
		if (0 == s) {
			if (_family == AF_INET) {
				address = ADDR_WILCARD;
			} else {
				address = ADDR_WILCARD6;
			}
		}
	}

	s = setsockopt(_d, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
	if (s != 0) {
		return -1;
	}

	if (_family == AF_INET6) {
		struct sockaddr_in6 sin6;

		s = SockAddr::CREATE_ADDR6(&sin6, address, port);
		if (s < 0) {
			return -1;
		}

		s = ::bind(_d, (struct sockaddr *) &sin6, SockAddr::IN6_SIZE);
	} else {
		struct sockaddr_in sin;

		s = SockAddr::CREATE_ADDR(&sin, address, port);
		if (s < 0) {
			return -1;
		}

		s = ::bind(_d, (struct sockaddr *) &sin, SockAddr::IN_SIZE);
	}
	if (s < 0) {
		return -1;
	}

	_status = O_RDWR | O_SYNC;
	s	= _name.copy_raw(address);

	return s;
}

/**
 * @method		: SockServer::listen
 * @param		:
 *	> queue_len	: length of queue for client connections.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: listen for socket connections and limit the queue
 * of incoming connections to 'queue_len'.
 */
int SockServer::listen(const unsigned int queue_len)
{
	return ::listen(_d, queue_len);
}

/**
 * @method		: SockServer::bind_listen
 * @param		:
 *	> address	: IP address or host name to listen to.
 *	> port		: port for accepting client connection.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *	create a socket that accepting client connection at 'address' and
 *	'port'.
 */
int SockServer::bind_listen(const char* address, const int port)
{
	register int s;

	s = SockServer::bind(address, port);
	if (0 == s) {
		return SockServer::listen(0);
	}
	return -1;
}

/**
 * @method		: SockServer::accept
 * @return		:
 *	< Socket	: success, new client accepted.
 *	< NULL		: fail.
 * @desc		:
 *	This method is used by server socket for accepting a new client
 *	connection.
 */
Socket* SockServer::accept()
{
	socklen_t		client_addrlen;
	struct sockaddr_in	client_addr;
	Socket*			client		= NULL;

	client = new Socket();
	if (!client) {
		return NULL;
	}

	client_addrlen	= SockAddr::IN_SIZE;
	client->_d	= ::accept(_d, (struct sockaddr *) &client_addr
					, &client_addrlen);
	if (client->_d < 0) {
		delete client;
		return NULL;
	}

	inet_ntop(AF_INET, &client_addr.sin_addr, client->_name._v
		, client->_name._l);

	client->_status	= O_RDWR | O_SYNC;

	add_client(client);

	return client;
}

/**
:* @method		: SockServer::accept6
 * @return		:
 *	< Socket	: success, new client accepted.
 *	< NULL		: fail.
 * @desc		:
 *	This method is used by server socket for accepting a new client
 *	connection. This is a for IPv6 server socket.
 */
Socket* SockServer::accept6()
{
	socklen_t		client_addrlen;
	struct sockaddr_in6	client_addr;
	Socket*			client = NULL;

	client = new Socket();
	if (!client) {
		return NULL;
	}

	client_addrlen	= SockAddr::IN_SIZE;
	client->_d	= ::accept(_d, (struct sockaddr *) &client_addr
					, &client_addrlen);
	if (client->_d < 0) {
		delete client;
		return NULL;
	}

	inet_ntop(_family, &client_addr.sin6_addr, client->_name._v
		, client->_name._l);

	client->_status	= O_RDWR | O_SYNC;

	add_client(client);

	return client;
}

/**
 * @method		: SockServer::accept_conn
 * @return		:
 *	< Socket	: success, new client accepted.
 *	< NULL		: fail.
 * @desc		:
 *	This method is used by server socket for accepting a new client
 *	connection. This is a generic version.
 */
Socket* SockServer::accept_conn()
{
	Socket* client = NULL;

	if (_family == AF_INET6) {
		client = accept6();
	} else {
		client = accept();
	}
	if (client && LIBVOS_DEBUG) {
		printf("[vos::SockSrvr] accept_conn: from '%s'\n"
			, client->_name.v());
	}

	return client;
}

/**
 * @method		: SockServer::add_client
 * @param		:
 *	> client	: socket object.
 * @desc		: add client object to list of clients.
 */
void SockServer::add_client(Socket* client)
{
	if (!client) {
		return;
	}

	lock_client();

	if (!_clients) {
		_clients = client;
	} else {
		Socket* p = _clients;

		while (p->_next) {
			p = p->_next;
		}

		p->_next	= client;
		client->_prev	= p;
	}

	unlock_client();
}

/**
 * @method		: SockServer::remove_client
 * @param		:
 *	> client	: Socket object.
 * @desc		:
 *	remove client object pointed by 'client' from list of clients.
 */
void SockServer::remove_client(Socket* client)
{
	if (! client) {
		return;
	}

	lock_client();

	if (client == _clients) {
		_clients = _clients->_next;
		if (_clients) {
			_clients->_prev	= NULL;
		}
	} else {
		if (client->_prev) {
			client->_prev->_next = client->_next;
		}
		if (client->_next) {
			client->_next->_prev = client->_prev;
		}
	}

	client->_next = NULL;
	client->_prev = NULL;

	unlock_client();
}

} /* namespace::vos */
