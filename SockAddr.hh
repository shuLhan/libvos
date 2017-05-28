//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_SOCKET_ADDRESS_HH
#define _LIBVOS_SOCKET_ADDRESS_HH 1

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include "Buffer.hh"

namespace vos {

#define IP_NO  0
#define IP_V4   1
#define IP_V6   2
#define IP_V4_6 4

#define	AF_INETS	(AF_INET + AF_INET6)

/**
 * @class		: SockAddr
 * @attr		:
 *	- _t		: type of address (IPv4, IPv6, or both).
 *	- _in		: the internet address.
 *	- _in6		: the internet address for IPv6.
 *	- IN_SIZE	: static, size of 'struct sockaddr_in'.
 *	- IN6_SIZE	: static, size of 'struct sockaddr_in6'.
 * @desc		:
 *	module for handling socket address.
 */
class SockAddr : public Object {
public:
	SockAddr();
	~SockAddr();

	int set(const int type, const char* addr, const uint16_t port);
	void set_port(const int type, const uint16_t port);
	uint16_t get_port(const int type = AF_INET);
	const char* get_address(const int type = AF_INET);
	const char* chars();

	int			_t;
	struct sockaddr_in	_in;
	struct sockaddr_in6	_in6;

	static int INIT(SockAddr** o, const int type, const char* addr
			, const uint16_t port);
	static int IS_IPV4(const char* str);

	static int CREATE_ADDR(struct sockaddr_in* sin, const char* addr
				, const uint16_t port);
	static int CREATE_ADDR6(struct sockaddr_in6* sin6
				, const char* address, const uint16_t port);

	static unsigned int IN_SIZE;
	static unsigned int IN6_SIZE;

	static const char* __cname;
private:
	//
	// `_p_address` is a temporary buffer to hold address string when
	// calling `get_address()`.
	//
	char _p_address[INET6_ADDRSTRLEN];

	SockAddr(const SockAddr&);
	void operator=(const SockAddr&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
