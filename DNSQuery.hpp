/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_RESOLVER_DNSQUERY_HPP
#define	_LIBVOS_RESOLVER_DNSQUERY_HPP	1

#include <arpa/inet.h>
#include "DNS_rr.hpp"

namespace vos {

enum _DNS_DATA_SIZE {
	DNS_TCP_HDR_SIZE	= 2
,	DNS_HDR_SIZE		= 12
};

enum _DNS_DATA_POS {
	DNS_AUT_CNT_POS		= 8
,	DNS_ADD_CNT_POS		= 10
};

enum _DNS_HDR_RCODE {
	RCODE_OK	= 0x0000
,	RCODE_FORMAT	= 0x0001
,	RCODE_SERVER	= 0x0002
,	RCODE_NAME	= 0x0003
,	RCODE_NOT_IMPL	= 0x0004
,	RCODE_REFUSED	= 0x0005
,	RCODE_FLAG	= 0x000F
};

enum _DNS_HDR_RTYPE {
	RTYPE_RA	= 0x0080	/* Recursion Available */
,	RTYPE_RD	= 0x0100	/* Recursion Desired */
,	RTYPE_TC	= 0x0200	/* TrunCation */
,	RTYPE_AA	= 0x0400	/* Authoritative Answer */
,	RTYPE_FLAG	= 0x0780
};

enum _DNS_HDR_OPCODE {
	OPCODE_QUERY	= 0x0000
,	OPCODE_IQUERY	= 0x0800
,	OPCODE_STATUS	= 0x1000
,	OPCODE_FLAG	= 0x1800
};

enum _DNS_HDR_TYPE {
	HDR_IS_QUERY	= 0x0000
,	HDR_IS_RESPONSE	= 0x8000
};

enum _DNS_BFR_TYPE {
	BUFFER_IS_UDP	= 1
,	BUFFER_IS_TCP	= 2
};

enum _dnsq_do_type {
	DNSQ_DO_DATA_ONLY	= 0
,	DNSQ_DO_ALL		= 1
};

/**
 * @class		: DNSQuery
 * @attr		:
 *	- _id		: packet transaction ID.
 *	- _flag		: packet header status.
 *	- _n_qry	: number of query RR.
 *	- _n_ans	: number of answer RR.
 *	- _n_aut	: number of authority RR.
 *	- _n_add	: number of additional RR.
 *	- _q_type	: type of packet, if question.
 *	- _q_class	: class of packet, if question.
 *	- _name		: domain name that will be queried.
 *	- _bfr_type	: type of packet (UDP or TCP).
 *	- _rr_ans	: list of answers record.
 *	- _rr_aut	: list of authority record.
 *	- _rr_add	: list of additional record.
 *	- _rr_ans_p	: pointer to the first byte of answer record on
 *	                  buffer.
 *	- _rr_aut_p	: pointer to the first byte of authority record on
 *	                  buffer.
 *	- _rr_add_p	: pointer to the first byte of additional RR on
 *                        buffer.
 *	- _ans_ttl_max	: maximum TTL in all of RR answer.
 * @desc		: module for processing DNS packet.
 *
 *	DNSQuery is ALWAYS in UDP packet mode, if you want to send the packet
 *	using TCP you must convert it to TCP first.
 */
class DNSQuery : public Buffer {
public:
	DNSQuery();
	virtual ~DNSQuery();

	int set(const Buffer* bfr, const int type = BUFFER_IS_UDP);
	DNSQuery* duplicate ();

	int to_udp(const Buffer* tcp = NULL);
	int to_tcp(const Buffer* udp = NULL);

	void set_header (uint16_t id, uint16_t flag, uint16_t n_qry
			, uint16_t n_ans
			, uint16_t n_aut
			, uint16_t n_add);

	int create_question(const char* qname
				, const int type = QUERY_T_ADDRESS);

	int extract();
	int extract_header();
	int extract_question();
	DNS_rr* extract_rr(int* offset, const int last_type = 0);
	int extract_label(Buffer* label, const int bfr_off);

	int create_answer (const char* name
			, uint16_t type, uint16_t clas
			, uint32_t ttl
			, uint16_t data_len, const char* data);

	void remove_rr_aut();
	void remove_rr_add();

	void set_id(const int id);
	void set_rr_answer_ttl(unsigned int ttl = UINT_MAX);

	void reset(const int do_type = DNSQ_DO_DATA_ONLY);
	void dump(const int do_type = DNSQ_DO_DATA_ONLY);

	/* DNS HEADER Section */
	uint16_t	_id;
	uint16_t	_flag;
	uint16_t	_n_qry;
	uint16_t	_n_ans;
	uint16_t	_n_aut;
	uint16_t	_n_add;
	/* DNS QUESTION Section */
	uint16_t	_q_type;
	uint16_t	_q_class;
	Buffer		_name;
	/* DNS Buffer for question & answer */
	int		_bfr_type;
	/* DNS resource records */
	DNS_rr*		_rr_ans;
	DNS_rr*		_rr_aut;
	DNS_rr*		_rr_add;
	const char*	_rr_ans_p;
	const char*	_rr_aut_p;
	const char*	_rr_add_p;
	/* additional attributes */
	uint32_t	_ans_ttl_max;

	/* pointer to query with different type */
	DNSQuery*	_next;

	static int INIT(DNSQuery** o, const Buffer* bfr
			, const int type = BUFFER_IS_UDP);
	static void ADD (DNSQuery** first, DNSQuery* nu);
private:
	DNSQuery(const DNSQuery&);
	void operator=(const DNSQuery&);
};

} /* namespace::vos */

#endif
