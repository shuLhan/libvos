/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_RESOLVER_DNSQUERY_HPP
#define	_LIBVOS_RESOLVER_DNSQUERY_HPP	1

#include <arpa/inet.h>
#include "Socket.hpp"
#include "DNS_rr.hpp"

namespace vos {

enum _DNS_PACKET_SIZE_AND_POS {
	DNS_TCP_HDR_SIZE	= 2,
	DNS_HDR_ID_SIZE		= 2,
	DNS_QTYPE_SIZE		= 2,
	DNS_QCLASS_SIZE		= 2,
	DNS_CNT_SIZE		= 2,
	DNS_ANS_CNT_POS		= 6,
	DNS_AUT_CNT_POS		= 8,
	DNS_ADD_CNT_POS		= 10,
	DNS_HDR_SIZE		= 12
};

enum _DNS_HDR_RCODE {
	RCODE_OK	= 0x0000,
	RCODE_FORMAT	= 0x0001,
	RCODE_SERVER	= 0x0002,
	RCODE_NAME	= 0x0003,
	RCODE_NOT_IMPL	= 0x0004,
	RCODE_REFUSED	= 0x0005,
	RCODE_FLAG	= 0x000F
};

enum _DNS_HDR_RTYPE {
	RTYPE_RA	= 0x0080,	/* Recursion Available */
	RTYPE_RD	= 0x0100,	/* Recursion Desired */
	RTYPE_TC	= 0x0200,	/* TrunCation */
	RTYPE_AA	= 0x0400,	/* Authoritative Answer */
	RTYPE_FLAG	= 0x0780
};

enum _DNS_HDR_OPCODE {
	OPCODE_QUERY	= 0x0000,
	OPCODE_IQUERY	= 0x0800,
	OPCODE_STATUS	= 0x1000,
	OPCODE_FLAG	= 0x1800
};

enum _DNS_HDR_TYPE {
	HDR_IS_QUERY	= 0x0000,
	HDR_IS_RESPONSE	= 0x8000
};

enum _DNS_BFR_TYPE {
	BUFFER_IS_UDP	= 1,
	BUFFER_IS_TCP	= 2
};

enum _dnsq_do_type {
	DNSQ_DO_ALL		= 0,
	DNSQ_DO_EXCEPT_BUFFER	= 1
};

class DNSQuery {
public:
	DNSQuery();
	~DNSQuery();

	int init(const Buffer *bfr);
	int set_buffer(const Buffer *bfr, const int type);

	int extract();
	int extract_header();
	int extract_question();
	int extract_rr(DNS_rr **rr, const unsigned char *bfr_org,
			const unsigned char *bfr,
			const unsigned char **bfr_ret,
			const int last_type);
	int read_label(Buffer *label, const unsigned char *bfr_org,
			const unsigned char *bfr, const int bfr_off);

	void remove_rr_aut();
	void remove_rr_add();

	void set_id(const int id);
	void set_tcp_size(int size);

	void reset(const int do_type);
	void net_to_host();
	void dump(const int do_type);

	static int INIT(DNSQuery **o, const Buffer *bfr, const int type);

	/* DNS HEADER Section */
	uint16_t	_id;
	uint16_t	_flag;
	uint16_t	_n_qry;
	uint16_t	_n_ans;
	uint16_t	_n_aut;
	uint16_t	_n_add;
	/* DNS QUESTION Section */
	uint16_t	_type;
	uint16_t	_class;
	Buffer		_name;
	/* DNS Buffer for question & answer */
	int		_bfr_type;
	Buffer		*_bfr;
	/* DNS resource records */
	DNS_rr		*_rr_ans;
	DNS_rr		*_rr_aut;
	DNS_rr		*_rr_add;
	const char	*_rr_ans_p;
	const char	*_rr_aut_p;
	const char	*_rr_add_p;
private:
	DISALLOW_COPY_AND_ASSIGN(DNSQuery);
};

} /* namespace::vos */

#endif
