/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#include "DNSQuery.hpp"

namespace vos {

/**
 * @method	: DNSQuery::DNSQuery
 */
DNSQuery::DNSQuery() : Buffer()
,	_id(0)
,	_flag(0)
,	_n_qry(0)
,	_n_ans(0)
,	_n_aut(0)
,	_n_add(0)
,	_q_type(0)
,	_q_class(0)
,	_name()
,	_bfr_type(BUFFER_IS_UDP)
,	_rr_ans(NULL)
,	_rr_aut(NULL)
,	_rr_add(NULL)
,	_rr_ans_p(NULL)
,	_rr_aut_p(NULL)
,	_rr_add_p(NULL)
,	_ans_ttl_max(0)
{}

/**
 * @method	: DNSQuery::~DNSQuery
 */
DNSQuery::~DNSQuery()
{
	if (_rr_ans) {
		delete _rr_ans;
		_rr_ans = NULL;
	}
	if (_rr_aut) {
		delete _rr_aut;
		_rr_aut = NULL;
	}
	if (_rr_add) {
		delete _rr_add;
		_rr_add = NULL;
	}
}

/**
 * @method	: DNSQuery::set_buffer
 * @param	:
 *	> bfr	: pointer to DNS packet.
 *	> type	: type of DNS packet origin, TCP or UDP.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: set DNSQuery packet to 'bfr'.
 */
int DNSQuery::set(const Buffer* bfr, const int type)
{
	if (!bfr) {
		return -1;
	}
	if (type != BUFFER_IS_UDP && type != BUFFER_IS_TCP) {
		return -1;
	}
	if (_id) {
		reset();
	}

	_bfr_type = type;
	return copy(bfr);
}

/**
 * @method	: DNSQuery::to_udp
 * @param	:
 *	> tcp	: pointer to DNS packet from TCP socket or buffer.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: set content of DNSQuery buffer to UDP packet using packet
 * from TCP. If 'tcp' is nil then convert the current buffer for UDP packet.
 */
int DNSQuery::to_udp(const Buffer *tcp)
{
	register int s;

	if (!tcp) {
		if (_bfr_type == BUFFER_IS_UDP) {
			return 0;
		}

		_i -= DNS_TCP_HDR_SIZE;
		memmove(_v, &_v[DNS_TCP_HDR_SIZE], _i);
		_v[_i] = 0;
	} else {
		s = copy_raw(&tcp->_v[DNS_TCP_HDR_SIZE]
				, tcp->_i - DNS_TCP_HDR_SIZE);
		if (s < 0) {
			return -1;
		}
	}

	_bfr_type = BUFFER_IS_UDP;
	return 0;
}

/**
 * @method	: DNSQuery::to_tcp
 * @param	:
 *	> udp	: pointer to DNS packet from UDP socket or buffer.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: set content of DNSQuery buffer to TCP packet using packet
 * from UDP. If udp is nil then convert the current buffer for TCP packet.
 */
int DNSQuery::to_tcp(const Buffer* udp)
{
	register int	s;
	uint16_t	size;

	if (!udp) {
		if (_bfr_type == BUFFER_IS_TCP) {
			return 0;
		}

		s = shiftr(DNS_TCP_HDR_SIZE);
		if (s < 0) {
			return -1;
		}

		size = (uint16_t)(_i - DNS_TCP_HDR_SIZE);
		size = htons(size);
		memcpy(_v, &size, DNS_TCP_HDR_SIZE);
	} else {
		s = resize(udp->_i + DNS_TCP_HDR_SIZE);
		if (s < 0) {
			return -1;
		}
		reset();

		size = (uint16_t) udp->_i;
		memcpy(_v, &size, DNS_TCP_HDR_SIZE);
		_i = DNS_TCP_HDR_SIZE;

		s = append(udp);
		if (s < 0) {
			return -1;
		}
	}

	_bfr_type = BUFFER_IS_TCP;
	return 0;
}

void DNSQuery::set_header (uint16_t id, uint16_t flag, uint16_t n_qry
		, uint16_t n_ans
		, uint16_t n_aut
		, uint16_t n_add)
{
	_id		= htons (id);
	_flag		= htons (flag);
	_n_qry		= htons (n_qry);
	_n_ans		= htons (n_ans);
	_n_aut		= htons (n_aut);
	_n_add		= htons (n_add);

	memcpy(_v	, &_id		, 2);
	memcpy(&_v[2]	, &_flag	, 2);
	memcpy(&_v[4]	, &_n_qry	, 2);
	memcpy(&_v[6]	, &_n_ans	, 2);
	memcpy(&_v[8]	, &_n_aut	, 2);
	memcpy(&_v[10]	, &_n_add	, 2);
	_i = DNS_HDR_SIZE;

	_id		= ntohs(_id);
	_flag		= ntohs(_flag);
	_n_qry		= ntohs(_n_qry);
	_n_ans		= ntohs(_n_ans);
	_n_aut		= ntohs(_n_aut);
	_n_add		= ntohs(_n_add);
}

/**
 * @method	: DNSQuery::create_question
 * @param	:
 *	> qname	: a domain name.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create question packet data to be send to server.
 */
int DNSQuery::create_question(const char* qname, const int type)
{
	if (!qname) {
		return -1;
	}

	int	s;
	int	len;
	Buffer	label;

	reset(DNSQ_DO_ALL);

	set_header ((uint16_t) (rand() % 65536)
		, HDR_IS_QUERY | OPCODE_QUERY | RTYPE_RD
		, 1, 0, 0, 0);

	_q_type		= htons((uint16_t) type);
	_q_class	= htons(QUERY_C_IN);

	_name.copy_raw(qname);

	len = _name._i + 16;

	if (len > _l) {
		s = resize(len);
		if (s < 0) {
			return -1;
		}
	}

	_i = DNS_HDR_SIZE;
	append_dns_label (qname, strlen (qname));

	append_bin(&_q_type, 2);
	append_bin(&_q_class, 2);

	_q_type		= ntohs(_q_type);
	_q_class	= ntohs(_q_class);

	return 0;
}

/**
 * @method	: DNSQuery::extract
 * @return	:
 *	< 0	: success, or buffer is empty.
 *	< -1	: fail.
 * @desc	: extract contents of buffer (DNS packet).
 */
int DNSQuery::extract()
{
	if (is_empty()) {
		return 0;
	}

	int		s		= 0;
	int		i		= 0;
	int		len		= 0;
	int		rr_type		= 0;
	DNS_rr*		rr		= NULL;

	if (_bfr_type == BUFFER_IS_TCP) {
		s = to_udp();
		if (s < 0) {
			return -1;
		}
	}

	reset(DNSQ_DO_DATA_ONLY);

	s = extract_header();
	if (s < 0) {
		return -1;
	}

	len = extract_question();
	if (len <= 0) {
		return -1;
	}

	_rr_ans_p = &_v[len];

	for (i = 0; i < _n_ans; ++i) {
		rr = extract_rr(&len, rr_type);
		if (!rr) {
			return -1;
		}

		rr_type	= rr->_type;
		DNS_rr::ADD(&_rr_ans, rr);
	}

	_rr_aut_p = &_v[len];
	for (i = 0; i < _n_aut; ++i) {
		rr = extract_rr(&len, 0);
		if (!rr) {
			return -1;
		}
		DNS_rr::ADD(&_rr_aut, rr);
	}

	_rr_add_p = &_v[len];
	for (i = 0; i < _n_add; ++i) {
		rr = extract_rr(&len, 0);
		if (!rr) {
			return -1;
		}
		DNS_rr::ADD(&_rr_add, rr);
	}

	return 0;
}

/**
 * @method	: DNSQuery::extract_header
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: extract header of DNS packet.
 */
int DNSQuery::extract_header()
{
	if (_i < DNS_HDR_SIZE) {
		return -1;
	}

	memcpy(&_id	, _v		, 2);
	memcpy(&_flag	, &_v[2]	, 2);
	memcpy(&_n_qry	, &_v[4]	, 2);
	memcpy(&_n_ans	, &_v[6]	, 2);
	memcpy(&_n_aut	, &_v[8]	, 2);
	memcpy(&_n_add	, &_v[10]	, 2);

	_id	= ntohs(_id);
	_flag	= ntohs(_flag);
	_n_qry	= ntohs(_n_qry);
	_n_ans	= ntohs(_n_ans);
	_n_aut	= ntohs(_n_aut);
	_n_add	= ntohs(_n_add);

	return 0;
}

/**
 * @method	: DNSQuery::extract_question
 * @return	:
 *	< >0	: success, length of question, from header + label + type +
 *	class.
 *	< -1	: fail.
 * @desc	: extract question data from buffer.
 */
int DNSQuery::extract_question()
{
	int startp	= DNS_HDR_SIZE;
	int len		= 0;

	if (startp > _i) {
		return -1;
	}

	_name.reset();
	len = extract_label(&_name, startp);
	if (len < 0) {
		return -1;
	}
	startp += len;

	len = startp + 4;
	if (len > _i) {
		return -1;
	}

	memcpy(&_q_type, &_v[startp], 2);
	startp += 2;

	memcpy(&_q_class, &_v[startp], 2);

	_q_type		= ntohs(_q_type);
	_q_class	= ntohs(_q_class);

	return len;
}

/**
 * @method		: DNSQuery::extract_rr
 * @param:
 *	> offset	: pointer to position of buffer after RR has been
 *                        extracted.
 *	> type		: type of the last extracted rr.
 * @return:
 *	< DNS_rr*	: success.
 *	< NULL		: fail.
 * @desc		: extract Resource-Record (RR) from buffer 'bfr'.
 */
DNS_rr* DNSQuery::extract_rr(int* offset, const int last_type)
{
	int	s	= 0;
	DNS_rr*	rr	= NULL;

	rr = new DNS_rr();
	if (!rr) {
		return NULL;
	}

	/* Get RR NAME */
	if (last_type != QUERY_T_MX) {
		s = extract_label(&rr->_name, *offset);
		if (s < 0) {
			goto err;
		}
		rr->_name_len	= (uint16_t) s;
		*offset		+= s;
	}

	/* Check if buffer is wide enough to have rr type */
	s = *offset + 2;
	if (s > _i) {
		return rr;
	}

	/* Get RR TYPE (2 bytes) */
	memcpy(&rr->_type, &_v[*offset], 2);
	rr->_type	= ntohs(rr->_type);
	*offset		+= 2;

	/* Check if buffer is wide enough to have rr class */
	s = *offset + 2;
	if (s > _i) {
		return rr;
	}

	/* Get RR CLASS (2 bytes) */
	memcpy(&rr->_class, &_v[*offset], 2);
	rr->_class	= ntohs(rr->_class);
	*offset		+= 2;

	/* Check 4 byte TTL */
	s = *offset + 4;
	if (s > _i) {
		return rr;
	}

	/* Get RR TTL */
	memcpy(&rr->_ttl, &_v[*offset], 4);
	rr->_ttl	= ntohl(rr->_ttl);
	*offset		+= 4;

	switch (rr->_type) {
	case QUERY_T_ADDRESS:
	case QUERY_T_NAMESERVER:
	case QUERY_T_CNAME:
	case QUERY_T_SRV:
		if (rr->_ttl > _ans_ttl_max) {
			_ans_ttl_max = rr->_ttl;
		}
		break;
	}

	/* Check 2 bytes for DATALEN */
	s = *offset + 2;
	if (s > _i) {
		return rr;
	}

	/* Get RR DATALEN */
	memcpy(&rr->_len, &_v[*offset], 2);
	rr->_len	= ntohs(rr->_len);
	*offset		+= 2;

	/* Check if packet length is less equal than DATALEN */
	s = *offset + rr->_len;
	if (s > _i) {
		return rr;
	}

	/* Check if buffer size is enough, if not resize it DATALEN */
	if (rr->_len > rr->_l) {
		s = rr->resize(rr->_len);
		if (s < 0) {
			goto err;
		}
	}

	/* Get RR DATA */
	memcpy(rr->_v, &_v[*offset], rr->_len);
	rr->_i = rr->_len;
	rr->_v[rr->_i] = 0;

	switch (rr->_type) {
	case QUERY_T_ADDRESS:
		inet_ntop(AF_INET, rr->_v, rr->_data._v, rr->_data._l);
		rr->_data._i = (int) strlen(rr->_data._v);
		*offset	+= 4;
		break;

	case QUERY_T_NAMESERVER:
	case QUERY_T_CNAME:
		s = extract_label(&rr->_data, *offset);
		if (s != rr->_len) {
			goto err;
		}
		*offset	+= s;
		break;

	case QUERY_T_SOA:
		s = extract_label(&rr->_data, *offset);
		if (s < 0) {
			goto err;
		}
		*offset += s;

		s = extract_label(&rr->_data2, *offset);
		if (s < 0) {
			goto err;
		}
		*offset += s;

		if ((*offset + 20) < _i) {
			goto err;
		}

		memcpy(&rr->_serial, &_v[*offset], 4);
		rr->_serial	= ntohl(rr->_serial);
		*offset		+= 4;

		memcpy(&rr->_refresh, &_v[*offset], 4);
		rr->_refresh	= ntohl(rr->_refresh);
		*offset		+= 4;

		memcpy(&rr->_retry, &_v[*offset], 4);
		rr->_retry	= ntohl(rr->_retry);
		*offset		+= 4;

		memcpy(&rr->_expire, &_v[*offset], 4);
		rr->_expire	= ntohl(rr->_expire);
		*offset		+= 4;

		memcpy(&rr->_minimum, &_v[*offset], 4);
		rr->_minimum	= ntohl(rr->_minimum);
		*offset		+= 4;
		break;

	case QUERY_T_PTR:
		s = extract_label(&rr->_data, *offset);
		if (s != rr->_len) {
			goto err;
		}
		*offset += s;
		break;

	case QUERY_T_HINFO:
		s = extract_label(&rr->_data, *offset);
		if (s < 0) {
			goto err;
		}
		*offset += s;

		s = extract_label(&rr->_data2, *offset);
		if (s < 0) {
			goto err;
		}
		*offset += s;
		break;

	case QUERY_T_MX:
		memcpy(&rr->_priority, &_v[*offset], 2);
		rr->_priority	= ntohs(rr->_priority);
		*offset		+= 2;

		s = extract_label(&rr->_data, *offset);
		if (s < 0) {
			goto err;
		}
		*offset += s;
		break;

	case QUERY_T_TXT:
		s = extract_label(&rr->_data, *offset);
		if (s != rr->_len) {
			goto err;
		}
		*offset += s;
		break;

	case QUERY_T_SRV:
		memcpy(&rr->_priority, &_v[*offset], 2);
		rr->_priority	= ntohs(rr->_priority);
		*offset		+= 2;

		memcpy(&rr->_weight, &_v[*offset], 2);
		rr->_weight	= ntohs(rr->_weight);
		*offset		+= 2;

		memcpy(&rr->_port, &_v[*offset], 2);
		rr->_port	= ntohs(rr->_port);
		*offset		+= 2;

		s = extract_label(&rr->_data, *offset);
		if (s < 0) {
			goto err;
		}
		*offset += s;
		break;

	default:
		fprintf(stderr
, "[vos::DNSQuery] extract_rr: (%s) Record type '%d' is not handle yet!\n"
		, _name.v(), rr->_type);
		goto err;
	}

	return rr;
err:
	if (LIBVOS_DEBUG) {
		fprintf(stderr
	, "[vos::DNSQuery] extract_rr: error extracting record type '%d'!"
	, rr->_type);
	}
	delete rr;
	return NULL;
}

/**
 * @method		: DNSQuery::extract_label
 * @param		:
 *	> label		: return value, content of DNS packet.
 *	> bfr_off	: buffer offset.
 * @return		:
 *	> >=0		: success, length of label.
 *	< -1		: fail.
 * @desc		: read a label from DNS packet buffer.
 */
int DNSQuery::extract_label(Buffer* label, const int bfr_off)
{
	int		len	= 0;
	int		ret_len	= 0;
	uint16_t	offset	= 0;
	const char*	p	= (const char*)&_v[bfr_off];

	/* Check buffer offset overflow */
	if (bfr_off > _i) {
		return -1;
	}

	while (*p) {
		/* Bitmask to check for POINTER */
		if ((*p & 0xC0) == 0xC0) {
			/* If we never used pointer before */
			if (!offset) {
				ret_len += 2;
			}

			/* Get POINTER */
			memcpy(&offset, p, 2);
			offset = ntohs(offset);
			offset &= 0x3FFF;
			if (offset > _i) {
				return -1;
			}

			/* Jump to POINTER offset */
			p = (const char*) &_v[offset];

		/* If we never get length before */
		} else if (len == 0) {
			/* Get LENGTH of message (1 byte) */
			len = *p;
			p++;

			/* If this is the last message LENGTH == 0 */
			if (label->_i > 0) {
				label->appendc('.');
			}
			if (!offset) {
				ret_len += len + 1;
			}
		} else {
			/* Pick each character by LENGTH */
			while (*p && len > 0) {
				label->appendc(*p);
				--len;
				p++;
			}
		}
	}
	if (!offset) {
		ret_len++;
	}

	return ret_len;
}

/**
 @method	: DNSQuery::create_answer
 @param		:
 > name		: host name.
 > type		: RR type.
 > clas		: RR class.
 > ttl		: time to live value for record.
 > data_len	: length of data.
 > data		: data.
 @return	:
 < 0		: success.
 < -1		: fail.
 @desc		: Create packet of DNS answer for hostname 'hname' with list of
	address in 'addrs'.
 */
int DNSQuery::create_answer (const char* name
				, uint16_t type, uint16_t clas
				, uint32_t ttl
				, uint16_t data_len, const char* data)
{
	uint16_t v = 0;

	reset (DNSQ_DO_ALL);

	_bfr_type	= BUFFER_IS_UDP;
	_name.copy_raw (name);
	_q_type		= type;
	_q_class	= clas;

	set_header (0
		, HDR_IS_RESPONSE | OPCODE_QUERY | RTYPE_RD
		, 1, (uint16_t) 1, 0, 0);

	/* Create question section */
	append_dns_label (name, strlen (name));

	v = htons (_q_type);
	append_bin (&v, 2);

	v = htons (_q_class);
	append_bin (&v, 2);

	/* Create answer section */
	_rr_ans = DNS_rr::INIT (name, type, clas, ttl, data_len, data);

	append_raw (_rr_ans->_v, _rr_ans->_i);

	return 0;
}

/**
 * @method	: DNSQuery::remove_rr_aut
 * @desc	: remove authority record from buffer.
 *
 *	buffer MUST be extracted before calling this function, using
 *	extract() method.
 */
void DNSQuery::remove_rr_aut()
{
	if (is_empty() || !_rr_aut_p) {
		return;
	}

	int rr_add_len;
	int rr_aut_len;

	if (_n_add > 0 && _rr_add_p) {
		rr_aut_len = (int)(_rr_add_p - _rr_aut_p);
		rr_add_len = (int)(&_v[_i] - _rr_add_p);

		memmove((void *)_rr_aut_p, _rr_add_p, rr_add_len);
		_i -= rr_aut_len;
	} else {
		_i = (int)(_rr_aut_p - _v);
		if (_i < 0) {
			_i = 0;
		}
	}
	if (_rr_aut) {
		delete _rr_aut;
		_rr_aut = NULL;
	}
	_i++;
	_rr_aut_p	= NULL;
	_v[_i]		= 0;
	_n_aut		= 0;
	memset(&_v[DNS_AUT_CNT_POS], 0, 2);
}

/**
 * @method	: DNSQuery::remove_rr_add
 * @desc	: remove additional record from buffer.
 *
 *	buffer MUST be extracted before calling this function, using
 *	extract() method.
 */
void DNSQuery::remove_rr_add()
{
	if (is_empty() || !_rr_add_p) {
		return;
	}

	_i = (int)(_rr_add_p - _v);
	if (_i < 0) {
		_i = 0;
	}
	if (_rr_add) {
		delete _rr_add;
		_rr_add = NULL;
	}

	_i++;
	_rr_add_p	= NULL;
	_v[_i]		= 0;
	_n_add		= 0;
	memset(&_v[DNS_ADD_CNT_POS], 0, 2);
}

/**
 * @method	: DNSQuery::set_id
 * @param	:
 *	> id	: a new identifier for DNS packet.
 * @desc	: set a new transaction ID for DNS packet buffer.
 */
void DNSQuery::set_id(const int id)
{
	if (is_empty()) {
		return;
	}

	_id = htons((uint16_t) id);

	memset(_v, 0, 2);
	memcpy(_v, &_id, 2);

	_id = (uint16_t) id;
}

/**
 * @method	: DNSQuery::set_rr_answer_ttl
 * @param	:
 *	> ttl	: time to live, in seconds. Default to UINT_MAX.
 * @desc	: set TTL value in each RR Answer record to 'ttl'. DNSQuery
 * object must be extracted before calling this function.
 */
void DNSQuery::set_rr_answer_ttl(unsigned int ttl)
{
	int	len	= 0;
	DNS_rr* p	= _rr_ans;

	ttl = htonl(ttl);

	while (p) {
		if (p->_type == QUERY_T_ADDRESS
		||  p->_type == QUERY_T_CNAME
		||  p->_type == QUERY_T_NAMESERVER) {
			len += p->_name_len + 4;

			memset((void*) &_rr_ans_p[len], 0, 4);
			memcpy((void*) &_rr_ans_p[len], &ttl, 4);

			len += 6 + p->_len;
		} else {
			len += p->_name_len + 10 + p->_len;
		}

		p = p->_next;
	}
}

/**
 * @method		: DNSQuery::reset
 * @param		:
 *	> do_type	: reset type.
 * @desc		: reset DNS packet buffer with reset mode is defined
 *                        by 'do_type'.
 */
void DNSQuery::reset(const int do_type)
{
	_id		= 0;
	_flag		= 0;
	_n_qry		= 0;
	_n_ans		= 0;
	_n_aut		= 0;
	_n_add		= 0;
	_q_type		= 0;
	_q_class	= 0;
	_bfr_type	= BUFFER_IS_UDP;
	_name.reset();

	if (do_type == DNSQ_DO_ALL) {
		Buffer::reset();
	}

	if (_rr_ans) {
		delete _rr_ans;
		_rr_ans	= NULL;
	}
	if (_rr_aut) {
		delete _rr_aut;
		_rr_aut = NULL;
	}
	if (_rr_add) {
		delete _rr_add;
		_rr_add = NULL;
	}
	_rr_ans_p	= NULL;
	_rr_aut_p	= NULL;
	_rr_add_p	= NULL;
}

/**
 * @method	: DNSQuery::dump
 * @desc	: print content of DNS packet buffer to standard output.
 */
void DNSQuery::dump(const int do_type)
{
	printf("\n[vos::DNSQuery] dump:\n; Buffer\n");

	if (_v && !(do_type & DNSQ_DO_DATA_ONLY)) {
		dump_hex();
	}

	printf("; HEADER section\n");
	printf(" id              : %d\n", _id);
	printf(" flag            : %#4X\n", _flag);
	printf("   query type    : %d\n", _flag & OPCODE_FLAG);
	printf("   response type : %#4X\n", _flag & RTYPE_FLAG);
	printf("   response code : %d\n", _flag & RCODE_FLAG);
	printf(" n question      : %d\n", _n_qry);
	printf(" n answer        : %d\n", _n_ans);
	printf(" n auth          : %d\n", _n_aut);
	printf(" n additional    : %d\n", _n_add);
	printf("; QUESTION section\n");
	printf(" type            : %d\n", _q_type);
	printf(" class           : %d\n", _q_class);
	printf(" name            : %s\n", _name.v());

	if (_rr_ans) {
		printf("\n; ANSWER section\n");
		_rr_ans->dump();
	}
	if (_rr_aut) {
		printf("\n; AUTHENTICATION section\n");
		_rr_aut->dump();
	}
	if (_rr_add) {
		printf("\n; ADDITIONAL section\n");
		_rr_add->dump();
	}
}

/**
 * @method	: DNSQuery::INIT
 * @param	:
 *	> o	: return value, pointer to DNSQuery object.
 *	> bfr	: pointer to DNS packet buffer.
 *	> type	: type of DNS packet (UDP or TCP).
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: initialize DNSQuery object 'o' using 'bfr' value.
 */
int DNSQuery::INIT(DNSQuery **o, const Buffer *bfr, const int type)
{
	if (!bfr) {
		return -1;
	}

	register int s = -1;

	(*o) = new DNSQuery();
	if (!(*o)) {
		return -1;
	}

	if (type == BUFFER_IS_TCP) {
		s = (*o)->to_udp(bfr);
	} else {
		s = (*o)->copy(bfr);
	}
	if (s != 0) {
		delete (*o);
		(*o) = NULL;
		return -1;
	}

	(*o)->_bfr_type = BUFFER_IS_UDP;

	return 0;
}

} /* namespace::vos */
