/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "DNS_rr.hpp"

namespace vos {

unsigned int DNS_rr::DNS_RDATA_MAX_SIZE = 255;

/**
 * @method	: DNS_rr::DNS_rr
 */
DNS_rr::DNS_rr() :
	_type(0),
	_class(0),
	_ttl(0),
	_len(0),
	_mx_pref(0),
	_name(),
	_data(),
	_next(NULL)
{}

/**
 * @method	: DNS_rr::~DNS_rr
 */
DNS_rr::~DNS_rr()
{
	if (_next)
		delete _next;
}

/**
 * @method	: DNS_rr::init
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: initialize DNS_rr object data.
 */
int DNS_rr::init()
{
	register int s;

	s = _name.init(NULL);
	if (s < 0)
		return s;

	s = _data.init_size(DNS_RDATA_MAX_SIZE);

	return s;
}

/**
 * @method	: DNS_rr::reset
 * @desc	: reset DNS RR data.
 */
void DNS_rr::reset()
{
	_type	= 0;
	_class	= 0;
	_ttl	= 0;
	_len	= 0;
	_name.reset();
	_data.reset();
	if (_next)
		delete _next;
}

/**
 * @method	: DNS_rr::dump
 * @desc	: print content of DNS_rr buffer to standard output.
 */
void DNS_rr::dump()
{
	DNS_rr *p = this;

	printf(" name|type|class|TTL|length|MX preference|data\n");
	while (p) {
		printf(" %s|%d|%d|%d|%d|%d|%s\n", p->_name._v, p->_type,
			p->_class, p->_ttl, p->_len, p->_mx_pref, p->_data._v);

		p = p->_next;
	}
}

/**
 * @method	: DNS_rr::ADD
 * @param	:
 *	> root	: head of list.
 *	> rr	: a new node.
 * @desc	: add a new node 'rr' to the list of 'root'.
 */
void DNS_rr::ADD(DNS_rr **root, DNS_rr *rr)
{
	if (! (*root)) {
		(*root) = rr;
	} else {
		DNS_rr *p = (*root);
		while (p->_next)
			p = p->_next;

		p->_next = rr;
	}
}

} /* namespace::vos */
