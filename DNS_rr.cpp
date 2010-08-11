/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "DNS_rr.hpp"

namespace vos {

unsigned int DNS_rr::RDATA_MAX_SIZE = 255;

/**
 * @method	: DNS_rr::DNS_rr
 */
DNS_rr::DNS_rr(const int bfr_size) : Buffer(bfr_size)
,	_type(0)
,	_class(0)
,	_ttl(0)
,	_len(0)
,	_name_len(0)
,	_name()
,	_data(RDATA_MAX_SIZE)
,	_data2(RDATA_MAX_SIZE)
,	_serial(0)
,	_refresh(0)
,	_retry(0)
,	_expire(0)
,	_minimum(0)
,	_mx_pref(0)
,	_next(NULL)
{}

/**
 * @method	: DNS_rr::~DNS_rr
 */
DNS_rr::~DNS_rr()
{
	if (_next) {
		delete _next;
	}
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
	reset();
	if (_next) {
		delete _next;
	}
}

/**
 * @method	: DNS_rr::dump
 * @desc	: print content of DNS_rr buffer to standard output.
 */
void DNS_rr::dump()
{
	Buffer	o;
	DNS_rr* p = this;

	o.append_raw("[vos::DNS_rr__] dump:\n");
	while (p) {
		o.aprint("\n[RR]\n"		\
			" name       : %s\n"	\
			" type       : %d\n"	\
			" class      : %d\n"	\
			" TTL        : %d\n"	\
			" length     : %d\n"	\
			" data       :\n", p->_name.v(), p->_type, p->_class
			, p->_ttl, p->_len);

		switch (p->_type) {
		case QUERY_T_ADDRESS:
			o.aprint(" address    : %s\n", p->_data.v());
			break;
		case QUERY_T_NAMESERVER:
			o.aprint(" nameserver : %s\n", p->_data.v());
			break;
		case QUERY_T_CNAME:
			o.aprint(" c. name    : %s\n", p->_data.v());
			break;
		case QUERY_T_SOA:
			o.aprint(" mname      : %s\n", p->_data.v());
			o.aprint(" rname      : %s\n", p->_data2.v());
			o.aprint(" serial     : %d\n", p->_serial);
			o.aprint(" refresh    : %d\n", p->_refresh);
			o.aprint(" retry      : %d\n", p->_retry);
			o.aprint(" expire     : %d\n", p->_expire);
			o.aprint(" minimum    : %d\n", p->_minimum);
			break;
		case QUERY_T_PTR:
			o.aprint(" PTRDNAME   : %s\n", p->_data.v());
			break;
		case QUERY_T_HINFO:
			o.aprint(" CPU        : %s\n", p->_data.v());
			o.aprint(" OS         : %s\n", p->_data2.v());
			break;
		case QUERY_T_MX:
			o.aprint(" score      : %d\n", p->_mx_pref);
			o.aprint(" exchange   : %s\n", p->_data.v());
			break;
		case QUERY_T_TXT:
			o.aprint(" TXT        : %s\n", p->_data.v());
			break;
		}
		p = p->_next;
	}
	printf("%s", o.v());
}

/**
 * @method	: DNS_rr::ADD
 * @param	:
 *	> root	: head of list.
 *	> rr	: a new node.
 * @desc	: add a new node 'rr' to the list of 'root'.
 */
void DNS_rr::ADD(DNS_rr** root, DNS_rr* rr)
{
	if (!rr) {
		return;
	}
	if (! (*root)) {
		(*root) = rr;
	} else {
		DNS_rr* p = (*root);

		while (p->_next) {
			p = p->_next;
		}
		p->_next = rr;
	}
}

} /* namespace::vos */
