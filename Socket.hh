//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_SOCKET_HH
#define _LIBVOS_SOCKET_HH 1

#include <sys/socket.h>
#include <pthread.h>
#include "File.hh"
#include "SockAddr.hh"

namespace vos {

/**
 * @class			: Socket
 * @attr			:
 *	- _family		: group or namespace this socket belong to.
 *	- _type			: type of socket (SOCK_STREAM, SOCK_DGRAM)
 *	- DFLT_BUFFER_SIZE	: static, default size of buffer object.
 * @desc			:
 *	module for handling Socket.
 */
class Socket : public File {
public:
	Socket(const int bfr_size = DFLT_BUFFER_SIZE);
	~Socket();

	int create(const int family = PF_INET, const int type = SOCK_STREAM);

	int set_socket_opt (int optname, int optval);
	int set_keep_alive (int val = 1);
	int set_reuse_address (int val = 1);

	inline int create_udp()
	{
		return Socket::create(PF_INET, SOCK_DGRAM);
	}

	int connect_to(struct sockaddr_in* sin);
	int connect_to6(struct sockaddr_in6* sin6);
	int connect_to_raw(const char* address, const uint16_t port);

	long int send_udp(struct sockaddr_in* addr, Buffer *bfr = NULL);
	long int send_udp_raw(struct sockaddr_in* addr
				, const char* bfr = NULL, int len = 0);
	long int recv_udp(struct sockaddr_in* addr);

	int		_family;
	int		_type;
	socklen_t	_recv_addr_l;

	static unsigned int DFLT_BUFFER_SIZE;
private:
	Socket(const Socket&);
	void operator=(const Socket&);
};

}

#endif
// vi: ts=8 sw=8 tw=78:
