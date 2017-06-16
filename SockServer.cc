//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SockServer.hh"

namespace vos {

const char* SockServer::__cname = "SockServer";

const char* SockServer::ADDR_WILCARD	= "0.0.0.0";
const char* SockServer::ADDR_WILCARD6	= "::";

SockServer::SockServer() : Socket()
,	_timeout()
,	_clients(NULL)
{}

SockServer::~SockServer()
{	
	if (_clients) {
		delete _clients;
		_clients = NULL;
	}
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
int SockServer::bind(const char* address, const uint16_t port)
{
	int	s	= 0;
	int	optval	= 1;

	if (LIBVOS_DEBUG) {
		printf("[%s] bind: %s:%d\n", __cname, address, port);
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

	_status = FILE_OPEN_SOCK;

	Error err = _name.copy_raw(address);
	if (err != NULL) {
		return -1;
	}

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
	return ::listen(_d, int(queue_len));
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
int SockServer::bind_listen(const char* address, const uint16_t port)
{
	int s = SockServer::bind(address, port);
	if (0 == s) {
		return SockServer::listen(2048);
	}
	return -1;
}

/**
 * Method accept_conn(client) will accept a new client connection and save the
 * `client` as Socket object.
 *
 * On success it will return NULL, otherwise it will return Error.
 */
Error SockServer::accept_conn(Socket** client)
{
	(*client) = new Socket();
	if (!(*client)) {
		return ErrOutOfMemory;
	}

	Error err = (*client)->accept(_d);
	if (err != NULL) {
		return err;
	}

	if ((*client) && LIBVOS_DEBUG) {
		printf("[%s] accept_conn: from '%s'\n", __cname
			, (*client)->name());
	}

	return NULL;
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
	if (!_clients) {
		_clients = new List();
	}

	_clients->push_tail(client);
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

	_clients->remove(client);
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
