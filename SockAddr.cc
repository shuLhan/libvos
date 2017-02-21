//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SockAddr.hh"

namespace vos {

unsigned int SockAddr::IN_SIZE	= sizeof(struct sockaddr_in);
unsigned int SockAddr::IN6_SIZE	= sizeof(struct sockaddr_in6);

/**
 * @method	: SockAddr::SockAddr
 * @desc	: SockAddr object constructor.
 */
SockAddr::SockAddr() : Object()
,	_t(0)
,	_in()
,	_in6()
,	_p_address()
{}

/**
 * @method	: SockAddr::~SockAddr
 * @desc	: SockAddr object destructor.
 */
SockAddr::~SockAddr()
{}

//
// `set_port()` will set port in address `type`.
//
int SockAddr::set_port(const int type, const uint16_t port)
{
	switch (type) {
	case AF_INET:
		_in.sin_port = htons(port);
		break;
	case AF_INET6:
		_in6.sin6_port = htons(port);
		break;
	case AF_INETS:
		_in.sin_port	= htons(port);
		_in6.sin6_port	= htons(port);
		break;
	}

	return 0;
}

/**
 * @method	: SockAddr::set
 * @param	:
 *	> type	: type of network address to be set.
 *	> addr	: network address in string representation.
 *	> port	: port number.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: set one of the network address using 'addr' and 'port'.
 */
int SockAddr::set(const int type, const char* addr, const uint16_t port)
{
	if (!addr) {
		return set_port(type, port);
	}

	register int s;

	switch (type) {
	case AF_INET:
		s = CREATE_ADDR(&_in, addr, port);
		break;
	case AF_INET6:
		s = CREATE_ADDR6(&_in6, addr, port);
		break;
	default:
		fprintf(stderr, "[vos::SockAddr] set: invalid type '%d'\n"
			, type);
		s = -1;
	}

	_t |= type;

	return s;
}

/**
 * @method	: SockAddr::get_port
 * @param	:
 *	> type	: type of network address.
 * @return	:
 *	< >=0	: any port number.
 * @desc	: return port number on specific network address.
 */
uint16_t SockAddr::get_port(const int type)
{
	switch (type) {
	case AF_INET:
		return ntohs(_in.sin_port);
	case AF_INET6:
	case AF_INETS:
		return ntohs(_in6.sin6_port);
	}
	return 0;
}

/**
 * @method		: SockAddr::get_address
 * @param		:
 *	> type		: type of network address.
 * @return		:
 *	> const char*	: string representation of network address.
 * @desc		: get a a string representation of network address.
 */
const char* SockAddr::get_address(const int type)
{
	const char* p = NULL;

	switch (type) {
	case AF_INET:
		p = inet_ntop(AF_INET, &_in.sin_addr, _p_address
				, INET6_ADDRSTRLEN);
		break;
	case AF_INET6:
	case AF_INETS:
		p = inet_ntop(AF_INET6, &_in6.sin6_addr, _p_address
				, INET6_ADDRSTRLEN);
		break;
	}
	return p;
}

//
// `chars()` return string format of this object.
//
const char* SockAddr::chars()
{
	if (__str) {
		free(__str);
		__str = NULL;
	}
	Buffer b;

	b.copy_raw(get_address());
	b.appendc(':');
	b.appendi(get_port());

	b.appendc(' ');

	b.append_raw(get_address(AF_INET6));
	b.appendc(':');
	b.appendi(get_port(AF_INET6));

	__str = b._v;
	b._v = NULL;

	return __str;
}

/**
 * @method	: SockAddr::init
 * @param	:
 *	> o	: return value, SockAddr object.
 *	> addr	: hostname or IP address.
 *	> port	: port number.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create and initialize initialize SockAddr object.
 */
int SockAddr::INIT(SockAddr** o, const int type, const char* addr
			, const uint16_t port)
{
	if (!addr) {
		return -1;
	}

	(*o) = new SockAddr();
	if (!(*o)) {
		return -1;
	}
	return (*o)->set(type, addr, port);
}

/**
 * @method	: SockAddr::IS_IPV4
 * @param	:
 *	> str	: string to check for.
 * @return	:
 *	< 1	: true.
 *	< 0	: false.
 * @desc	: check if 'str' is IPv4 address and it is valid address.
 */
int SockAddr::IS_IPV4(const char* str)
{
	if (!str) {
		return 0;
	}

	register int	n	= 0;
	register int	dot	= 0;
	char		x[3];

	while (*str) {
		if (isdigit(*str)) {
			if (n >= 3) {
				return 0;
			}
			x[n] = *str;
			n++;
		} else if (*str == '.') {
			if (n == 0) {
				return 0;
			}
			if (n == 3) {
				if (x[0] > '2') {
					return 0;
				}
				if (x[0] == '2') {
					if (x[1] > '5') {
						return 0;
					}
					if (x[1] == '5') {
						if (x[2] > '5') {
							return 0;
						}
					}
				}
			}
			n = 0;
			dot++;
		} else {
			return 0;
		}
		str++;
	}
	if (dot != 3 || n == 0) {
		return 0;
	}
	if (n == 3) {
		if (x[0] > '2') {
			return 0;
		}
		if (x[0] == '2') {
			if (x[1] > '5') {
				return 0;
			}
			if (x[1] == '5') {
				if (x[2] > '5') {
					return 0;
				}
			}
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
 *	< -1	: fail.
 * @desc	:
 *	create IPv4 socket address object from address 'addr' and port
 *	'port', with address family default to network (AF_INET).
 */
int SockAddr::CREATE_ADDR(struct sockaddr_in* sin, const char* addr
			, const uint16_t port)
{
	if (!sin || !addr) {
		return -1;
	}

	int			s;
	int			err	= 0;
	struct hostent		he;
	struct hostent*		hep	= NULL;
	Buffer			ip_addr;

	memset(sin, 0, IN_SIZE);

	if (IS_IPV4(addr)) {
		s = inet_pton(AF_INET, addr, &sin->sin_addr);
		if (s <= 0) {
			return -1;
		}
	} else {
		ip_addr.resize(512);
		do {
			s = gethostbyname2_r(addr, AF_INET, &he, ip_addr._v
						, ip_addr._l, &hep, &err);
			if (ERANGE == s) {
				ip_addr.resize(ip_addr._l * 2);
			}
		} while (ERANGE == s);

		if (err) {
			return -1;
		}

		memcpy(&sin->sin_addr, hep->h_addr, hep->h_length);
	}

	sin->sin_family	= AF_INET;
	sin->sin_port	= htons(port);

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
 *	< -1	: fail.
 * @desc	:
 *	create IPv6 network address using 'address' and 'port'.
 */
int SockAddr::CREATE_ADDR6(struct sockaddr_in6* sin6
			, const char* addr, const uint16_t port)
{
	if (!sin6 || !addr) {
		return -1;
	}

	int		s;
	int		err	= 0;
	struct hostent	he;
	struct hostent*	hep	= NULL;
	char*		buf	= NULL;
	Buffer		ip_addr;

	memset(sin6, 0, IN6_SIZE);

	buf = (char *) strchr(addr, ':');
	if (buf) {
		s = inet_pton(AF_INET6, addr, &sin6->sin6_addr);
		if (s <= 0) {
			return -1;
		}
	} else {
		ip_addr.resize(512);
		do {
			s = gethostbyname2_r(addr, AF_INET6, &he, ip_addr._v
						, ip_addr._l, &hep, &err);
			if (ERANGE == s) {
				ip_addr.resize(ip_addr._l * 2);
			}
		} while (ERANGE == s);

		if (err) {
			return -1;
		}

		memcpy(&sin6->sin6_addr, hep->h_addr, hep->h_length);
	}

	sin6->sin6_family	= AF_INET6;
	sin6->sin6_port		= htons(port);

	return 0;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
