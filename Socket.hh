//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_SOCKET_HH
#define _LIBVOS_SOCKET_HH 1

#include <sys/socket.h>
#include <pthread.h>
#include "File.hh"
#include "SockAddr.hh"
#include "Locker.hh"

namespace vos {

enum SocketConnType {
	IS_UDP = 0
,	IS_TCP = 1
};


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
	static const char* __CNAME;
	static size_t DFLT_BUFFER_SIZE;

	explicit Socket(const size_t bfr_size = DFLT_BUFFER_SIZE);
	~Socket();

	Error accept(int server_fd);
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
				, const char* bfr = NULL, size_t len = 0);
	long int recv_udp(struct sockaddr_in* addr);

protected:
	int		_family;
	int		_type;
	socklen_t	_recv_addr_l;

private:
	Socket(const Socket&);
	void operator=(const Socket&);

	Locker _locker;
};

}

#endif
// vi: ts=8 sw=8 tw=80:
