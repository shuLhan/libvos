/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "DNSQuery.hpp"

namespace vos {

/**
 * @method	: DNSQuery::DNSQuery
 */
DNSQuery::DNSQuery() :
	_id(0),
	_flag(0),
	_n_qry(0),
	_n_ans(0),
	_n_aut(0),
	_n_add(0),
	_type(0),
	_class(0),
	_name(),
	_bfr_type(BUFFER_IS_UDP),
	_bfr(NULL),
	_rr_ans(NULL),
	_rr_aut(NULL),
	_rr_add(NULL),
	_rr_ans_p(NULL),
	_rr_aut_p(NULL),
	_rr_add_p(NULL)
{}

/**
 * @method	: DNSQuery::~DNSQuery
 */
DNSQuery::~DNSQuery()
{
	if (_bfr) {
		delete _bfr;
	}
	if (_rr_ans) {
		delete _rr_ans;
	}
	if (_rr_aut) {
		delete _rr_aut;
	}
	if (_rr_add) {
		delete _rr_add;
	}
}

/**
 * @method	: DNSQuery::init
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: initialize DNSQuery object.
 */
int DNSQuery::init(const Buffer *bfr)
{
	register int s;

	s = _name.init(NULL);
	if (0 == s) {
		s = Buffer::INIT(&_bfr, bfr);
	}

	return s;
}

/**
 * @method	: DNSQuery::set_buffer
 * @param	:
 *	> bfr	: pointer to Buffer object.
 *	> type	: type of DNS packet buffer (UDP or TCP).
 * @return	:
 *	< 0	: success, or 'bfr' is null.
 *	< <0	: fail.
 * @desc	: set content of DNSQuery buffer to 'bfr'.
 */
int DNSQuery::set_buffer(const Buffer *bfr, int type)
{
	if (!bfr)
		return 0;

	register int s;

	if (!_bfr) {
		s = Buffer::INIT(&_bfr, bfr);
	} else {
		s = _bfr->copy(bfr);
	}
	_bfr_type = type;

	return s;
}

/**
 * @method	: DNSQuery::extract
 * @return	:
 *	< 0	: success, or buffer is empty.
 *	< <0	: fail.
 * @desc	: extract contents of buffer (DNS packet).
 */
int DNSQuery::extract()
{
	int			s		= 0;
	int			i		= 0;
	int			len		= 0;
	int			rr_type		= 0;
	const unsigned char	*bfr_org	= NULL;
	const unsigned char	*p		= NULL;
	const unsigned char	*ret		= NULL;
	DNS_rr			*rr		= NULL;

	reset(DNSQ_DO_EXCEPT_BUFFER);

	if (_bfr->is_empty())
		return 0;

	bfr_org	= (unsigned char *) _bfr->_v;
	p	= bfr_org;

	len = extract_header();
	if (len <= 0) {
		return -1;
	}
	p += len;

	len = extract_question();
	if (len <= 0) {
		return -1;
	}

	p += len;

	if (_bfr_type == BUFFER_IS_TCP) {
		bfr_org += 2;
	}

	_rr_ans_p = (const char *) p;
	for (i = 0; i < _n_ans; ++i) {
		s = extract_rr(&rr, bfr_org, p, &ret, rr_type);
		if (s != 0) {
			return s;
		}
		p = ret;
		if (rr) {
			rr_type = rr->_type;
			DNS_rr::ADD(&_rr_ans, rr);
			rr = NULL;
		}
	}

	_rr_aut_p = (const char *) p;
	for (i = 0; i < _n_aut; ++i) {
		s = extract_rr(&rr, bfr_org, p, &ret, 0);
		if (s != 0) {
			return s;
		}
		p = ret;
		if (rr) {
			DNS_rr::ADD(&_rr_aut, rr);
			rr = NULL;
		}
	}

	_rr_add_p = (const char *) p;
	for (i = 0; i < _n_add; ++i) {
		s = extract_rr(&rr, bfr_org, p, &ret, 0);
		if (s != 0) {
			return s;
		}
		p = ret;
		if (rr) {
			DNS_rr::ADD(&_rr_add, rr);
			rr = NULL;
		}
	}

	return 0;
}

/**
 * @method	: DNSQuery::extract_header
 * @return	:
 *	< 0	: buffer is empty.
 *	< >0	: success, length of header.
 *	< <0	: fail.
 * @desc	: extract header of DNS packet.
 */
int DNSQuery::extract_header()
{
	int len = DNS_HDR_SIZE;

	if (!_bfr)
		return 0;

	if (len > _bfr->_i)
		return -1;

	if (_bfr_type == BUFFER_IS_TCP) {
		len += DNS_TCP_HDR_SIZE;
		if (len > _bfr->_i) {
			return -1;
		}
		memcpy(this, _bfr->_v + DNS_TCP_HDR_SIZE, DNS_HDR_SIZE);
	} else {
		memcpy(this, _bfr->_v, DNS_HDR_SIZE);
	}

	_id	= ntohs(_id);
	_flag	= ntohs(_flag);
	_n_qry	= ntohs(_n_qry);
	_n_ans	= ntohs(_n_ans);
	_n_aut	= ntohs(_n_aut);
	_n_add	= ntohs(_n_add);

	return len;
}

/**
 * @method	: DNSQuery::extract_question
 * @return	:
 *	< 0	: buffer is empty.
 *	< >0	: success, length of question, label + type + class.
 *	< <0	: fail.
 * @desc	: extract question data from buffer.
 */
int DNSQuery::extract_question()
{
	int			startp	= 0;
	int			len	= 0;
	const unsigned char	*bfr	= NULL;

	if (!_bfr)
		return 0;

	startp = DNS_HDR_SIZE;
	if (startp > _bfr->_i)
		return -1;

	bfr = (unsigned char *) _bfr->_v;

	_name.reset();
	if (_bfr_type == BUFFER_IS_TCP) {
		startp += DNS_TCP_HDR_SIZE;
		if (startp > _bfr->_i)
			return -1;

		len = read_label(&_name, bfr, bfr + startp, 0);
	} else {
		len = read_label(&_name, bfr, bfr + startp, 0);
	}
	startp += len;

	memcpy(&_type, bfr + startp, DNS_QTYPE_SIZE);
	_type	= ntohs(_type);
	len	+= DNS_QTYPE_SIZE;
	startp	+= DNS_QTYPE_SIZE;
	if (startp > _bfr->_i)
		return -1;

	memcpy(&_class, bfr + startp, DNS_QCLASS_SIZE);
	_class	= ntohs(_class);
	len	+= DNS_QCLASS_SIZE;

	return len;
}

/**
 * @method		: DNSQuery::extract_rr
 * @param:
 *	> rr		: return value, RR object after extracted.
 *	> bfr_org	: the original buffer, pointer to the original buffer
 *                        received from network.
 *	> bfr		: pointer to the part of original buffer.
 *	> bfr_ret	: pointer to position of buffer after RR has been
 *                        extracted.
 *	> type		: type of the last extracted rr.
 * @return:
 *	< 0		: success.
 *	< <0		: fail.
 * @desc		: extract Resource-Record (RR) from buffer 'bfr'.
 */
int DNSQuery::extract_rr(DNS_rr **rr, const unsigned char *bfr_org,
				const unsigned char *bfr,
				const unsigned char **bfr_ret,
				const int last_type)
{
	int	s	= 0;
	int	l	= 0;
	DNS_rr	*prr	= NULL;

	if (! (*rr)) {
		(*rr) = new DNS_rr();
		if (! (*rr))
			return -1;

		s = (*rr)->init();
		if (s < 0)
			return s;
	} else if (0 == last_type) {
		(*rr)->reset();
	}

	prr = (*rr);

	if (last_type != QUERY_T_MX) {
		l	= read_label(&prr->_name, bfr_org, bfr, 0);
		bfr	+= l;
	}

	memcpy(&prr->_type, bfr, 2);
	prr->_type	= ntohs(prr->_type);
	bfr		+= 2;

	memcpy(&prr->_class, bfr, 2);
	prr->_class	= ntohs(prr->_class);
	bfr		+= 2;

	memcpy(&prr->_ttl, bfr, 4);
	prr->_ttl	= ntohl(prr->_ttl);
	bfr		+= 4;

	memcpy(&prr->_len, bfr, 2);
	prr->_len	= ntohs(prr->_len);
	bfr		+= 2;

	switch (prr->_type) {
	case QUERY_T_ADDRESS:
		inet_ntop(AF_INET, bfr, prr->_data._v, prr->_data._l);
		bfr += prr->_len;
		break;

	case QUERY_T_CNAME:
	case QUERY_T_NAMESERVER:
		l	= read_label(&prr->_data, bfr_org, bfr, 0);
		bfr	+= prr->_len;
		break;

	case QUERY_T_MX:
		memcpy(&prr->_mx_pref, bfr, 2);
		prr->_mx_pref	= ntohs(prr->_mx_pref);
		bfr		+= 2;

		l	= read_label(&prr->_data, bfr_org, bfr, 0);
		bfr	+= prr->_len;
		break;

	default:
		fprintf(stderr, "[DNSQUERY] %s: Record type '%d' is not handle yet!\n",
			_name._v, prr->_type);
		delete prr;
		(*rr) = NULL;
		return -1;
	}

	(*bfr_ret) = bfr;

	return 0;
}

/**
 * @method		: DNSQuery::read_label
 * @param		:
 *	> label		: return value, content of DNS packet.
 *	> bfr_org	: the original buffer, pointer to the original DNS
 *                        packet buffer.
 *	> bfr		: pointer to the part of original buffer, usually
 *                        passed by user in process of extracting RR.
 *	> bfr_off	: buffer offset.
 * @return		:
 *	> >=0		: success, ength of label.
 *	< <0		: fail.
 * @desc		: read a label from DNS packet buffer.
 */
int DNSQuery::read_label(Buffer *label, const unsigned char *bfr_org,
				const unsigned char *bfr, const int bfr_off)
{
	int			len	= 0;
	int			ret	= 0;
	uint16_t		offset	= 0;
	const unsigned char	*p	= &bfr[bfr_off];

	while (*p) {
		if ((*p & 0xC0) == 0xC0) {
			memcpy(&offset, p, 2);
			offset = ntohs(offset);
			offset &= 0x3FFF;

			p = &bfr_org[offset];

			if (! ret)
				ret = 2;
		} else if (len == 0) {
			len = *p;
			*p++;

			if (ret) {
				label->appendc('.');
			}
			if (! offset)
				ret += len + 1;
		} else {
			label->appendc(*p);
			--len;
			*p++;
		}
	}

	if (! offset)
		ret++;

	return ret;
}

/**
 * @method	: DNSQuery::remove_rr_aut
 * @desc	: remove authority record from buffer.
 *
 *	buffer '_bfr' MUST be extracted before calling this function, using
 *	extract() method.
 */
void DNSQuery::remove_rr_aut()
{
	if (!_bfr)
		return;
	if (!_rr_aut_p)
		return;

	_bfr->_i		= _rr_aut_p - _bfr->_v;
	if (_bfr->_i < 0) {
		_bfr->_i	= 0;
	}
	_bfr->_v[_bfr->_i]	= '\0';
	_rr_aut_p		= NULL;
	_n_aut			= 0;

	if (_rr_aut) {
		delete _rr_aut;
		_rr_aut = NULL;
	}

	if (_bfr_type == BUFFER_IS_TCP) {
		memset(&_bfr->_v[DNS_TCP_HDR_SIZE + DNS_AUT_CNT_POS], '\0',
			DNS_CNT_SIZE);
	} else {
		memset(&_bfr->_v[DNS_AUT_CNT_POS], '\0', DNS_CNT_SIZE);
	}
}

/**
 * @method	: DNSQuery::remove_rr_add
 * @desc	: remove additional record from buffer.
 *
 *	buffer '_bfr' MUST be extracted before calling this function, using
 *	extract() method.
 */
void DNSQuery::remove_rr_add()
{
	if (!_bfr)
		return;
	if (!_rr_add_p)
		return;

	_bfr->_i		= _rr_add_p - _bfr->_v;
	if (_bfr->_i < 0) {
		_bfr->_i	= 0;
	}
	_bfr->_v[_bfr->_i]	= '\0';
	_rr_add_p		= NULL;
	_n_add			= 0;

	if (_rr_add) {
		delete _rr_add;
		_rr_add = NULL;
	}

	if (_bfr_type == BUFFER_IS_TCP) {
		memset(&_bfr->_v[DNS_TCP_HDR_SIZE + DNS_ADD_CNT_POS], '\0',
			DNS_CNT_SIZE);
	} else {
		memset(&_bfr->_v[DNS_ADD_CNT_POS], '\0', DNS_CNT_SIZE);
	}
}

/**
 * @method	: DNSQuery::set_id
 * @param	:
 *	> id	: a new identifier for DNS packet.
 * @desc	: set a new transaction ID for DNS packet buffer.
 */
void DNSQuery::set_id(const int id)
{
	_id = htons(id);

	if (!_bfr)
		return;

	if (_bfr_type == BUFFER_IS_UDP) {
		memset(_bfr->_v, '\0', DNS_HDR_ID_SIZE);
		memcpy(_bfr->_v, &_id, DNS_HDR_ID_SIZE);
	} else {
		memset(&_bfr->_v[DNS_TCP_HDR_SIZE], '\0', DNS_HDR_ID_SIZE);
		memcpy(&_bfr->_v[DNS_TCP_HDR_SIZE], &_id, DNS_HDR_ID_SIZE);
	}
}

/**
 * @method	: DNSQuery::set_tcp_size
 * @param	:
 *	> size	: a new size for DNS TCP packet.
 * @desc	: set size field in DNS packet, only work if DNS packet is
 *                from TCP connection.
 */
void DNSQuery::set_tcp_size(int size)
{
	if (_bfr_type != BUFFER_IS_TCP)
		return;

	size = htons(size);
	memset(_bfr->_v, '\0', DNS_TCP_HDR_SIZE);
	memcpy(_bfr->_v, &size, DNS_TCP_HDR_SIZE);
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
	_type		= 0;
	_class		= 0;
	_name.reset();
	if (_bfr && !(do_type & DNSQ_DO_EXCEPT_BUFFER)) {
		_bfr_type = BUFFER_IS_UDP;
		_bfr->reset();
	}

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
 * @method	: DNSQuery::net_to_host
 * @desc	: convert all integer value in DNS packet from network to host
 *                byte order.
 */
void DNSQuery::net_to_host()
{
	_id	= ntohs(_id);
	_flag	= ntohs(_flag);
	_n_qry	= ntohs(_n_qry);
	_n_ans	= ntohs(_n_ans);
	_n_aut	= ntohs(_n_aut);
	_n_add	= ntohs(_n_add);
	_type	= ntohs(_type);
	_class	= ntohs(_class);
}

/**
 * @method	: DNSQuery::dump
 * @desc	: print content of DNS packet buffer to standard output.
 */
void DNSQuery::dump(const int do_type)
{
	printf("\n; Buffer\n");
	if (_bfr && !(do_type & DNSQ_DO_EXCEPT_BUFFER))
		_bfr->dump_hex();

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
	printf(" type            : %d\n", _type);
	printf(" class           : %d\n", _class);
	printf(" name            : %s\n", _name._v);

	if (_rr_ans) {
		printf("; ANSWER section\n");
		_rr_ans->dump();
	}
	if (_rr_aut) {
		printf("; AUTHENTICATION section\n");
		_rr_aut->dump();
	}
	if (_rr_add) {
		printf("; ADDITIONAL section\n");
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
 *	> 0	: success.
 *	> <0	: fail.
 * @desc	: initialize DNSQuery object 'o' using 'bfr' value.
 */
int DNSQuery::INIT(DNSQuery **o, const Buffer *bfr, const int type)
{
	register int s = -1;

	(*o) = new DNSQuery();
	if ((*o)) {
		(*o)->_bfr_type = type;

		s = (*o)->init(bfr);
		if (s != 0) {
			delete (*o);
			(*o) = NULL;
		}
	}
	return s;
}

} /* namespace::vos */
