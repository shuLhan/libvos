/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Socket.hpp"

namespace vos {

unsigned int Socket::DFLT_BUFFER_SIZE = 65536;

/**
 * @method		: Socket::Socket
 * @param		:
 *	> bfr_size	: size of socket buffer.
 * @desc		: Socket object constructor.
 */
Socket::Socket(const int bfr_size) : File(bfr_size)
,	_family(0)
,	_type(0)
,	_next(NULL)
,	_prev(NULL)
{
	_name.resize(INET6_ADDRSTRLEN);
}

/**
 * @method	: Socket::~Socket
 * @desc	: Socket object destructor.
 */
Socket::~Socket()
{}

/**
 * @method		: Socket::create
 * @param		:
 *	> family	: address family (AF_LOCAL, AF_UNIX, AF_FILE, AF_INET,
 *				AF_INET6).
 *	> type		: socket type (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW).
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: create a new socket descriptor.
 */
int Socket::create(const int family, const int type)
{
	_d = ::socket(family, type, 0);
	if (_d < 0) {
		return -1;
	}

	_family	= family;
	_type	= type;

	if (type == SOCK_STREAM) {
		_status = FILE_OPEN_NO;
	} else {
		_status = FILE_OPEN_RW | O_SYNC;
	}

	return 0;
}

/**
 * @method	: Socket::connect_to
 * @param	:
 *	> sin	: socket address internet.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: connect to end point as defined in sockaddr_in object 'sin'.
 */
int Socket::connect_to(struct sockaddr_in* sin)
{
	if (!sin) {
		return -1;
	}

	register int s;

	s = ::connect(_d, (struct sockaddr*) sin, SockAddr::IN_SIZE);
	if (s < 0) {
		return -1;
	}

	inet_ntop(AF_INET, &sin->sin_addr, _name._v, _name._l);
	_status	= O_RDWR | O_SYNC;

	return 0;
}

/**
 * @method	: Socket::connect_to6
 * @param	:
 *	> sin	: socket address internet.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: connect to end point address, through IPv6, as defined in
 * sockaddr_in6 object 'sin6'.
 */
int Socket::connect_to6(struct sockaddr_in6* sin6)
{
	if (!sin6) {
		return -1;
	}

	register int s;

	s = ::connect(_d, (struct sockaddr*) sin6, SockAddr::IN_SIZE);
	if (s < 0) {
		return -1;
	}

	inet_ntop(AF_INET6, &sin6->sin6_addr, _name._v, _name._l);
	_status	= O_RDWR | O_SYNC;

	return 0;
}

/**
 * @method		: Socket::connect_to_raw
 * @param		:
 *	> address	: destination hostname or IP address to connect to.
 *	> port		: destination port number.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: connect socket to 'address' with 'port'.
 */
int Socket::connect_to_raw(const char* address, const int port)
{
	if (!address) {
		return -1;
	}

	register int s;

	if (_family == AF_INET6) {
		struct sockaddr_in6 sin6;

		s = SockAddr::CREATE_ADDR6(&sin6, address, port);
		if (s < 0) {
			return -1;
		}

		s = ::connect(_d, (struct sockaddr *) &sin6
				, SockAddr::IN6_SIZE);
	} else {
		struct sockaddr_in sin;

		s = SockAddr::CREATE_ADDR(&sin, address, port);
		if (s < 0) {
			return -1;
		}

		s = ::connect(_d, (struct sockaddr *) &sin, SockAddr::IN_SIZE);
	}
	if (s < 0) {
		return -1;
	}

	_status	= O_RDWR | O_SYNC;
	s	= _name.copy_raw(address);

	return s;
}

/**
 * @method	: Socket::send_udp
 * @param	:
 *	> addr	: address of end point.
 *	> bfr	: buffer object to be send.
 * @return	:
 *	< >=0	: success, number of bytes send.
 *	< -1	: fail.
 * @desc	: send 'bfr' to 'addr' using datagram protocol.
 */
long int Socket::send_udp(struct sockaddr_in* addr, Buffer* bfr)
{
	if (!addr) {
		return -1;
	}

	register ssize_t s = 0;

	if (!bfr) {
		if (_i > 0) {
			s = ::sendto(_d, _v, _i, 0
				, (struct sockaddr *) addr, SockAddr::IN_SIZE);
		}
	} else if (bfr->_i > 0) {
			s = ::sendto(_d, bfr->_v, bfr->_i, 0
				, (struct sockaddr *) addr, SockAddr::IN_SIZE);
	}

	return s;
}

/**
 * @method	: Socket::send_udp_raw
 * @param	:
 *	> addr	: address of end point.
 *	> bfr	: buffer object to be send.
 * @return	:
 *	< >=0	: success, number of bytes send.
 *	< -1	: fail.
 * @desc	:
 *	send 'bfr' with length is 'len' to 'addr' using datagram protocol.
 */
long int Socket::send_udp_raw(struct sockaddr_in* addr, const char* bfr
				, int len)
{
	if (!addr) {
		return -1;
	}
	if (!bfr) {
		bfr = _v;
		len = _i;
	}
	if (!len) {
		len = (int) strlen(bfr);
	}

	return ::sendto(_d, bfr, len, 0, (struct sockaddr*) addr
			, SockAddr::IN_SIZE);
}

/**
 * @method	: Socket::recv_udp
 * @param	:
 *	> addr	: output, originating address of end point.
 * @return	:
 *	< >=0	: success, size of received data.
 *	< -1	: fail.
 * @desc	:
 *	received data from end point using datagram protocol and save the end
 *	point address to 'addr'.
 */
long int Socket::recv_udp(struct sockaddr_in* addr)
{
	if (!addr) {
		return 0;
	}

	socklen_t addr_len = SockAddr::IN_SIZE;

	_i = (int) ::recvfrom(_d, _v, _l, 0, (struct sockaddr*) addr, &addr_len);
	if (_i >= 0) {
		_v[_i] = '\0';
	}

	return _i;
}

} /* namespace::vos */
