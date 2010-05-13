/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
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
 *	- DFLT_BUFFER_SIZE	: static, default size of buffer object.
 *	- DFLT_LISTEN_SIZE	: static, default size of client queue.
 *	- DFLT_NAME_SIZE	: static, default length of hostname.
 *	- ADDR_WILCARD		: static, wilcard address for IPv4.
 *	- _family		: group or namespace this socket belong to.
 *	- _timeout		: time data used by socket as server.
 *	- _client_lock		: lock for accessing list of clients object.
 *	- _clients		: list of client connections.
 *	- _next			: pointer to the next client object.
 *	- _prev			: pointer to the previous client object.
 * @desc			:
 *	module for handling Socket.
 */
class Socket : public File {
public:
	Socket();
	~Socket();

	int init(const int bfr_size = Socket::DFLT_BUFFER_SIZE);
	void lock_client();
	void unlock_client();

	int create(const int family, const int type);
	int create_tcp();
	int create_udp();

	int bind(const char *address, const int port);
	int listen(const unsigned int queue_len);
	int bind_listen(const char *address, const int port);

	int connect_to(struct sockaddr_in *sin);
	int connect_to_raw(const char *address, const int port);

	void add_client_r(Socket *client);
	void remove_client(Socket *client);
	void remove_client_r(Socket *client);

	Socket * accept();
	Socket * accept6();
	Socket * accept_conn();

	int send(Buffer *bfr);
	int send_raw(const char *bfr, int len = 0);

	int send_udp(struct sockaddr_in *addr, Buffer *bfr);
	int send_udp_raw(struct sockaddr_in *addr, const char *bfr,
				const int len);
	int recv_udp(struct sockaddr_in *addr);

	static Socket * ADD_CLIENT(Socket *list, Socket *client);

	static unsigned int	DFLT_BUFFER_SIZE;
	static unsigned int	DFLT_LISTEN_SIZE;
	static unsigned int	DFLT_NAME_SIZE;
	static const char*	ADDR_WILCARD;

	int		_family;
	struct timeval	_timeout;
	pthread_mutex_t	_client_lock;
	Socket		*_clients;
	Socket		*_next;
	Socket		*_prev;
private:
	Socket(const Socket&);
	void operator=(const Socket&);
};

}

#endif
