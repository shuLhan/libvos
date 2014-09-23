/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_SOCKET_HPP
#define	_LIBVOS_SOCKET_HPP	1

#include <sys/socket.h>
#include <pthread.h>
#include "File.hpp"
#include "SockAddr.hpp"

namespace vos {

/**
 * @class			: Socket
 * @attr			:
 *	- _family		: group or namespace this socket belong to.
 *	- _type			: type of socket (SOCK_STREAM, SOCK_DGRAM)
 *	- _next			: pointer to the next client object.
 *	- _prev			: pointer to the previous client object.
 *	- DFLT_BUFFER_SIZE	: static, default size of buffer object.
 * @desc			:
 *	module for handling Socket.
 */
class Socket : public File {
public:
	Socket(const int bfr_size = DFLT_BUFFER_SIZE);
	~Socket();

	int create(const int family = PF_INET, const int type = SOCK_STREAM);

	inline int create_udp()
	{
		return Socket::create(PF_INET, SOCK_DGRAM);
	}

	int connect_to(struct sockaddr_in* sin);
	int connect_to6(struct sockaddr_in6* sin6);
	int connect_to_raw(const char* address, const int port);

	long int send_udp(struct sockaddr_in* addr, Buffer *bfr = NULL);
	long int send_udp_raw(struct sockaddr_in* addr
				, const char* bfr = NULL, int len = 0);
	long int recv_udp(struct sockaddr_in* addr);

	int		_family;
	int		_type;
	socklen_t	_recv_addr_l;
	Socket*		_next;
	Socket*		_prev;

	static unsigned int DFLT_BUFFER_SIZE;
private:
	Socket(const Socket&);
	void operator=(const Socket&);
};

}

#endif
