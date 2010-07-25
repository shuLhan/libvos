/*
 * Copyright (C) 2009,2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "SockAddr.hpp"

namespace vos {

unsigned int SockAddr::IN_SIZE	= sizeof(struct sockaddr_in);
unsigned int SockAddr::IN6_SIZE	= sizeof(struct sockaddr_in6);

/**
 * @method	: SockAddr::SockAddr
 * @desc	: SockAddr object constructor.
 */
SockAddr::SockAddr() :
	_addr(NULL),
	_in(NULL),
	_next(NULL),
	_last(this)
{}

/**
 * @method	: SockAddr::~SockAddr
 * @desc	: SockAddr object destructor.
 */
SockAddr::~SockAddr()
{
	if (_addr)
		delete _addr;
	if (_in)
		free(_in);
	if (_next)
		delete _next;
	_last = NULL;
}

/**
 * @method	: SockAddr::dump
 * @desc	: print content of SockAddr object to standard output.
 */
void SockAddr::dump()
{
	SockAddr *p = this;

	while (p) {
		printf(" address : %s\n", p->_addr
					? p->_addr->v()
					: "\0");
		p = p->_next;
	}
}

/**
 * @method	: SockAddr::init
 * @param	:
 *	> o	: return value, SockAddr object.
 *	> addr	: hostname or IP address.
 *	> port	: port number.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: create and initialize initialize SockAddr object.
 */
int SockAddr::INIT(SockAddr **o, const char *addr, const int port)
{
	register int s;

	(*o) = new SockAddr();
	if (!(*o))
		return -1;

	s = Buffer::INIT_RAW(&(*o)->_addr, addr);
	if (s < 0)
		return s;

	(*o)->_addr->trim();

	(*o)->_in = (struct sockaddr_in *) calloc(1, IN_SIZE);
	if (!(*o)->_in)
		return -1;

	if (addr) {
		s = CREATE_ADDR((*o)->_in, (*o)->_addr->_v, port);
		if (s < 0)
			return s;
	}

	return 0;
}

/**
 * @method	: SockAddr::ADD
 * @param	:
 *	> head	: pointer to the head of list.
 *	> node	: a new SockAddr object that will be added to list.
 * @desc	: add 'node' to the list of 'head'.
 */
void SockAddr::ADD(SockAddr **head, SockAddr *node)
{
	if (!(*head)) {
		(*head)		= node;
		(*head)->_last	= node;
	} else {
		(*head)->_last->_next	= node;
		(*head)->_last		= node;
	}
}

/**
 * @method	: SockAddr::IS_IPV4
 * @param	:
 *	> str	: string to check for.
 * @return	:
 *	< 1	: true.
 *	< 0	: false.
 * @desc	: check if 'str' is IPv4 address and it is valid address.
 *	Minimum length of IPv4 address is x.x.x.x == 7, and
 *	maximum length of IPv4 address is xxx.xxx.xxx.xxx == 15.
 *
 */
int SockAddr::IS_IPV4(const char *str)
{
	if (!str) {
		return 0;
	}
	register int	n	= 0;
	register int	dot	= 0;
	char		x[3];

	while (*str) {
		if (isdigit(*str)) {
			x[n] = *str;
			n++;
		} else if (*str == '.') {
			if (n == 0 || n > 4) {
				return 0;
			}
			if (n == 3) {
				if (x[0] > '2') {
					return 0;
				}
				if (x[1] > '5') {
					return 0;
				}
				if (x[2] > '5') {
					return 0;
				}
			}
			n = 0;
			dot++;
		} else {
			return 0;
		}
		str++;
	}
	if (dot != 3 || n == 0 || n > 4)  {
		return 0;
	}
	if (n == 3) {
		if (x[0] > '2') {
			return 0;
		}
		if (x[1] > '5') {
			return 0;
		}
		if (x[2] > '5') {
			return 0;
		}
	}

	return 1;
}

/**
 * @method	: SockAddr::CREATE_ADDR
 * @param	:
 *	> sin	: return value, struct sockaddr_in object.
 *	> addr	: hostname or IPv4 address.
 *	> port	: port number.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	:
 *	create a IPv4 socket address object from address 'addr' and port
 *	'port', with address family default to internet (AF_INET).
 */
int SockAddr::CREATE_ADDR(struct sockaddr_in *sin, const char *addr,
				const int port)
{
	int		s;
	int		buf_len	= 512;
	int		err	= 0;
	struct hostent	he;
	struct hostent	*hep	= NULL;
	char		*buf	= NULL;

	memset(sin, 0, SockAddr::IN_SIZE);

	sin->sin_family	= AF_INET;
	sin->sin_port	= htons((uint16_t) port);

	if (IS_IPV4(addr)) {
		s = inet_pton(AF_INET, addr, &sin->sin_addr);
		if (s <= 0) {
			return -1;
		}
	} else {
		do {
			buf = (char *) calloc(buf_len, sizeof(char));
			s = gethostbyname2_r(addr, AF_INET, &he, buf,
						buf_len, &hep, &err);
			if (ERANGE == s) {
				free(buf);
				buf_len *= 2;
			}
		} while (ERANGE == s);

		if (err) {
			free(buf);
			return -1;
		}

		memcpy(&sin->sin_addr, hep->h_addr, hep->h_length);
		free(buf);
	}
	return 0;
}

/**
 * @method	: SockAddr::CREATE_ADDR6
 * @param	:
 *	> sin6	: return value, struct sockaddr_in6 object.
 *	> addr	: hostname or IP address.
 *	> port	: port number.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	:
 *	create IPv6 internet address using 'address' and 'port'.
 */
int SockAddr::CREATE_ADDR6(struct sockaddr_in6 *sin6, const char *addr,
				const int port)
{
	int		s;
	int		buf_len	= 512;
	int		err	= 0;
	struct hostent	he;
	struct hostent	*hep	= NULL;
	char		*buf	= NULL;

	memset(sin6, 0, SockAddr::IN6_SIZE);

	sin6->sin6_family	= AF_INET6;
	sin6->sin6_port		= htons((uint16_t) port);

	buf = (char *) strchr(addr, ':');
	if (buf) {
		s = inet_pton(AF_INET6, addr, &sin6->sin6_addr);
		if (s <= 0) {
			return -1;
		}
	} else {
		do {
			buf = (char *) calloc(buf_len, sizeof(char));
			s = gethostbyname2_r(addr, AF_INET6, &he, buf,
						buf_len, &hep, &err);
			if (ERANGE == s) {
				free(buf);
				buf_len *= 2;
			}
		} while (ERANGE == s);

		if (err) {
			free(buf);
			return -1;
		}

		memcpy(&sin6->sin6_addr, hep->h_addr, hep->h_length);
		free(buf);
	}
	return 0;
}

} /* namespace::vos */
