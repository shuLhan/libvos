//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_SOCKET_ADDRESS_HH
#define _LIBVOS_SOCKET_ADDRESS_HH 1

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include "Buffer.hh"

namespace vos {

#define	AF_INETS	(AF_INET + AF_INET6)

/**
 * @class		: SockAddr
 * @attr		:
 *	- _t		: type of address (IPv4, IPv6, or both).
 *	- _in		: the internet address.
 *	- _in6		: the internet address for IPv6.
 *	- _next		: pointer to the next SockAddr in the list.
 *	- _last		: pointer to the last SockAddr in the list.
 *	- IN_SIZE	: static, size of 'struct sockaddr_in'.
 *	- IN6_SIZE	: static, size of 'struct sockaddr_in6'.
 * @desc		:
 *	module for handling socket address.
 */
class SockAddr : public Buffer {
public:
	SockAddr(const int bfr_size = INET6_ADDRSTRLEN);
	~SockAddr();

	int set(const int type, const char* addr, const int port);
	int get_port(const int type = AF_INET);
	const char* get_address(const int type = AF_INET);
	void dump();

	int			_t;
	struct sockaddr_in	_in;
	struct sockaddr_in6	_in6;
	SockAddr*		_next;
	SockAddr*		_last;

	static int INIT(SockAddr** o, const int type, const char* addr
			, const int port);
	static int IS_IPV4(const char* str);

	static int CREATE_ADDR(struct sockaddr_in* sin, const char* addr
				, const int port);
	static int CREATE_ADDR6(struct sockaddr_in6* sin6
				, const char* address, const int port);

	static void ADD(SockAddr** head, SockAddr *node);

	static unsigned int IN_SIZE;
	static unsigned int IN6_SIZE;
private:
	SockAddr(const SockAddr&);
	void operator=(const SockAddr&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
