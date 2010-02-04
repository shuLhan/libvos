/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_RESOLVER_DNS_RR_HPP
#define	_LIBVOS_RESOLVER_DNS_RR_HPP	1

#include <stdint.h>
#include "Buffer.hpp"

namespace vos {

enum _RR_TYPE {
	QUERY_T_ADDRESS		= 1,
	QUERY_T_NAMESERVER,
	QUERY_T_MAIL_DEST,
	QUERY_T_MAIL_FORWARDER,
	QUERY_T_CNAME,
	QUERY_T_SOA,
	QUERY_T_MAIL_BOX,
	QUERY_T_MAIL_GROUP,
	QUERY_T_MAIL_RENAME,
	QUERY_T_NULL,
	QUERY_T_WKS,
	QUERY_T_PTR,
	QUERY_T_HINFO,
	QUERY_T_MINFO,
	QUERY_T_MX,
	QUERY_T_TXT,
	QUERY_T_AXFR		= 252,
	QUERY_T_MAILB,
	QUERY_T_MAILA,
	QUERY_T_ALL
};

enum _RR_CLASS {
	QUERY_C_IN	= 1,
	QUERY_C_CS,
	QUERY_C_CH,
	QUERY_C_HS,
	QUERY_C_ALL	= 255
};

/**
 * @class	: DNS_rr
 * @desc	:
 *	submodule of DNSQuery, for listing Resource Record of DNS packet.
 */
class DNS_rr {
public:
	DNS_rr();
	~DNS_rr();

	int init();

	void reset();
	void dump();

	static void ADD(DNS_rr **root, DNS_rr *rr);

	uint16_t	_type;
	uint16_t	_class;
	uint32_t	_ttl;
	uint16_t	_len;
	uint16_t	_mx_pref;
	Buffer		_name;
	Buffer		_data;
	DNS_rr		*_next;
private:
	DNS_rr(const DNS_rr&);
	const operator=(const DNS_rr&);

	static unsigned int DNS_RDATA_MAX_SIZE;
};

} /* namespace::vos */

#endif
