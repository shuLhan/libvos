/**
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
#include "File.hpp"

namespace vos {

class Socket : public File {
public:
	static int		DFLT_BUFFER_SIZE;
	static unsigned int	DFLT_LISTEN_QUEUE;

	Socket(int buffer_size = DFLT_BUFFER_SIZE);
	~Socket();

	void create(const int family = AF_INET, const int type = SOCK_STREAM);
	void create_tcp();
	void create_udp();

	void create_addr(struct sockaddr_in *sin, const char *address,
				const int port);
	void create_addr6(struct sockaddr_in6 *sin6, const char *address,
				const int port);

	int bind(const char *address, const int port);
	int binds(const char *list_address, const int port_default);

	void listen(const unsigned int queue_len = DFLT_LISTEN_QUEUE);

	int bind_listen(const char *address, const int port);
	int binds_listen(const char *list_address, const int port_default);

	void connect_to(const char *address, const int port);

	void add_client(Socket *client);
	void remove_client(Socket *client);

	Socket * accept();
	Socket * accept6();
	Socket * accept_conn();

	void send(Buffer *bfr);

	int send_udp(struct sockaddr *addr, Buffer *bfr);
	int recv_udp(struct sockaddr *addr);

	int		_family;
	int		_port;
	fd_set		_readfds;
	fd_set		_writefds;
	fd_set		_errorfds;
	struct timeval	_timeout;
	Socket		*_clients;
	Socket		*_next;
	Socket		*_prev;
private:
	DISALLOW_COPY_AND_ASSIGN(Socket);	
};

}

#endif
