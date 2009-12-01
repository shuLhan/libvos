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

#define	SOCK_ADDR_WILCARD	"0.0.0.0"

class Socket : public File {
public:
	static int DFLT_BUFFER_SIZE;
	static int DFLT_LISTEN_SIZE;
	static int DFLT_NAME_SIZE;

	Socket();
	~Socket();

	int init(const int bfr_size);

	int create(const int family, const int type);
	int create_tcp();
	int create_udp();

	int create_addr(struct sockaddr_in *sin, const char *address,
				const int port);
	int create_addr6(struct sockaddr_in6 *sin6, const char *address,
				const int port);

	int bind(const char *address, const int port);
	int listen(const unsigned int queue_len);

	int bind_listen(const char *address, const int port);

	int connect_to(const char *address, const int port);

	void add_client(Socket *client);
	void remove_client(Socket *client);

	Socket * accept();
	Socket * accept6();
	Socket * accept_conn();

	int send(Buffer *bfr);
	int send_raw(const char *bfr, const int len);

	int send_udp(struct sockaddr *addr, Buffer *bfr);
	int send_udp_raw(struct sockaddr *addr, const char *bfr,
				const int len);
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
