/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "DNS_rr.hpp"

namespace vos {

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

DNS_rr::DNS_rr() :
	_type(0),
	_class(0),
	_ttl(0),
	_len(0),
	_name(),
	_data(DNS_RDATA_MAX_SIZE),
	_next(NULL)
{}

DNS_rr::~DNS_rr()
{
	if (_next)
		delete _next;
}

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

void DNS_rr::dump()
{
	DNS_rr *p = this;

	while (p) {
		printf(" RR name   : %s\n", p->_name._v);
		printf(" RR type   : %d\n", p->_type);
		printf(" RR class  : %d\n", p->_class);
		printf(" RR TTL    : %d\n", p->_ttl);
		printf(" RR length : %d\n", p->_len);
		printf(" RR data   : %s\n\n", p->_data._v);

		p = p->_next;
	}
}

} /* namespace::vos */
