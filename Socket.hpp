/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_SOCKET_HPP
#define	_LIBVOS_SOCKET_HPP	1

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include "File.hpp"

namespace vos {

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

	int connect_to(const char *address, const int port);

	void add_client_r(Socket *client);
	void remove_client(Socket *client);
	void remove_client_r(Socket *client);

	Socket * accept();
	Socket * accept6();
	Socket * accept_conn();

	int send(Buffer *bfr);
	int send_raw(const char *bfr, const int len);

	int send_udp(struct sockaddr *addr, Buffer *bfr);
	int send_udp_raw(struct sockaddr *addr, const char *bfr,
				const int len);
	int recv_udp(struct sockaddr *addr);

	static int IS_IPV4(const char *str);
	static int CREATE_ADDR(struct sockaddr_in *sin, const char *addr,
				const int port);
	static int CREATE_ADDR6(struct sockaddr_in6 *sin6, const char *address,
				const int port);

	static Socket * ADD_CLIENT(Socket *list, Socket *client);

	static unsigned int	DFLT_BUFFER_SIZE;
	static unsigned int	DFLT_LISTEN_SIZE;
	static unsigned int	DFLT_NAME_SIZE;
	static const char*	ADDR_WILCARD;

	int		_family;
	int		_port;
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
