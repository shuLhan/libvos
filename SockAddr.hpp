/*
 * Copyright (C) 2009,2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_SOCKET_ADDRESS_HPP
#define	_LIBVOS_SOCKET_ADDRESS_HPP	1

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include "Buffer.hpp"

namespace vos {

/**
 * @class		: SockAddr
 * @attr		:
 *	- IN_SIZE	: static, size of 'struct sockaddr_in'.
 *	- IN6_SIZE	: static, size of 'struct sockaddr_in6'.
 *	- _addr		: the readable address.
 *	- _in		: the internet address.
 *	- _next		: pointer to the next SockAddr in the list.
 *	- _last		: pointer to the last SockAddr in the list.
 * @desc		:
 *	module for handling socket address.
 */
class SockAddr {
public:
	SockAddr();
	~SockAddr();

	void dump();

	static int INIT(SockAddr **o, const char *addr, const int port);
	static void ADD(SockAddr **head, SockAddr *node);

	static int IS_IPV4(const char *str);
	static int CREATE_ADDR(struct sockaddr_in *sin, const char *addr,
				const int port);
	static int CREATE_ADDR6(struct sockaddr_in6 *sin6, const char *address,
				const int port);

	static unsigned int IN_SIZE;
	static unsigned int IN6_SIZE;

	Buffer			*_addr;
	struct sockaddr_in	*_in;
	SockAddr		*_next;
	SockAddr		*_last;
private:
	SockAddr(const SockAddr&);
	void operator=(const SockAddr&);
};

} /* namespace::vos */

#endif
