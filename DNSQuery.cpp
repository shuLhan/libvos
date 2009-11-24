/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "DNSQuery.hpp"

namespace vos {

DNSQuery::DNSQuery(Buffer *bfr) :
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
	_rr_add(NULL)
{
	if (bfr)
		_bfr = new Buffer(bfr);
}

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

int DNSQuery::extract(Buffer *bfr, int type)
{
	int		i	= 0;
	int		len	= 0;
	unsigned char	*p	= NULL;
	unsigned char	*ret	= NULL;
	DNS_rr		*rr	= NULL;

	if (! bfr) {
		bfr = _bfr;
	} else {
		if (! _bfr) {
			_bfr = new Buffer(bfr);
		} else {
			_bfr->copy(bfr);
		}
	}

	if (bfr->is_empty())
		return 0;

	len	= extract_buffer((unsigned char *) bfr->_v, _bfr->_i, type);
	p	= (unsigned char *) &bfr->_v[len];
	for (i = 0; i < _n_ans; ++i) {
		extract_rr(&rr, (unsigned char *) bfr->_v, p, &ret);
		p = ret;
		if (rr) {
			DNS_rr::ADD(&_rr_ans, rr);
			rr = NULL;
		}
	}

	for (i = 0; i < _n_aut; ++i) {
		extract_rr(&rr, (unsigned char *) bfr->_v, p, &ret);
		p = ret;
		if (rr) {
			DNS_rr::ADD(&_rr_aut, rr);
			rr = NULL;
		}
	}

	for (i = 0; i < _n_add; ++i) {
		extract_rr(&rr, (unsigned char *) bfr->_v, p, &ret);
		p = ret;
		if (rr) {
			DNS_rr::ADD(&_rr_add, rr);
			rr = NULL;
		}
	}

	return 0;
}

int DNSQuery::extract_buffer(unsigned char *bfr, unsigned int bfr_len,
				const int type)
{
	int ret	= 0;
	int len	= 0;

	if (! bfr)
		return 0;

	if (type == BUFFER_IS_TCP) {
		bfr	= bfr + 2;
		ret	+= 2;
	}

	memcpy(this, bfr, DNS_HEADER_SIZE);

	_id	= ::ntohs(_id);
	_flag	= ::ntohs(_flag);
	_n_qry	= ::ntohs(_n_qry);
	_n_ans	= ::ntohs(_n_ans);
	_n_aut	= ::ntohs(_n_aut);
	_n_add	= ::ntohs(_n_add);

	len	= read_label(&_name, bfr, bfr + DNS_HEADER_SIZE, 0);
	ret	+= DNS_HEADER_SIZE + len;
	bfr	+= DNS_HEADER_SIZE + len;

	memcpy(&_type, bfr, 2);
	_type = ::ntohs(_type);
	bfr += 2;

	memcpy(&_class, bfr, 2);
	_class = ::ntohs(_class);
	bfr += 2;

	ret += 4;

	return ret;
}

/**
 * @desc: extract Resource-Record (RR) from buffer 'bfr'.
 *
 * @param:
 *	> rr		: return value, RR object after extracted.
 *	> bfr_org	: the original buffer, pointer to the original buffer
 *			received from network.
 *	> bfr		: pointer to the part of original buffer.
 *	> bfr_ret	: pointer to position of buffer after RR has been
 *			extracted.
 *
 * @return:
 *	< 0	: success.
 *	< 1	: fail.
 */
int DNSQuery::extract_rr(DNS_rr **rr, unsigned char *bfr_org,
				unsigned char *bfr,
				unsigned char **bfr_ret)
{
	int	l	= 0;
	DNS_rr	*prr	= NULL;

	if (! (*rr)) {
		(*rr) = new DNS_rr();
		if (! (*rr))
			return 1;
	} else {
		(*rr)->reset();
	}

	prr = (*rr);

	l	= read_label(&prr->_name, bfr_org, bfr, 0);
	bfr	+= l;

	memcpy(&prr->_type, bfr, 2);
	prr->_type	= ::ntohs(prr->_type);
	bfr		+= 2;

	memcpy(&prr->_class, bfr, 2);
	prr->_class	= ::ntohs(prr->_class);
	bfr		+= 2;

	memcpy(&prr->_ttl, bfr, 4);
	prr->_ttl	= ::ntohl(prr->_ttl);
	bfr		+= 4;

	memcpy(&prr->_len, bfr, 2);
	prr->_len	= ::ntohs(prr->_len);
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
	default:
		fprintf(stderr, "[DNSQUERY] query type '%d' is not handle yet!\n", prr->_type);
		prr->reset();
	}

	(*bfr_ret) = bfr;

	return 0;
}

int DNSQuery::read_label(Buffer *label, unsigned char *bfr_org,
				unsigned char *bfr, int bfr_off)
{
	int		len	= 0;
	int		ret	= 0;
	uint16_t	offset	= 0;
	unsigned char	*p	= &bfr[bfr_off];

	while (*p) {
		if ((*p & 0xC0) == 0xC0) {
			memcpy(&offset, p, 2);
			offset = ::ntohs(offset);
			offset &= 0x3FFF;

			p = &bfr_org[offset];

			if (! ret)
				ret = 2;
		} else if (len == 0) {
			len = *p;
			++p;

			if (ret) {
				label->appendc('.');
			}
			if (! offset)
				ret += len + 1;
		} else {
			label->appendc(*p);
			--len;
			++p;
		}
	}

	if (! offset)
		ret++;

	return ret;
}

void DNSQuery::set_id(int id)
{
	_id = ::htons(id);

	if (! _bfr)
		return;

	memset(_bfr->_v, '\0', 2);
	memcpy(_bfr->_v, &_id, 2);

	_id = id;
}

void DNSQuery::reset()
{
	_id		= 0;
	_flag		= 0;
	_n_qry		= 0;
	_n_ans		= 0;
	_n_aut		= 0;
	_n_add		= 0;
	_type		= 0;
	_class		= 0;
	_bfr_type	= BUFFER_IS_UDP;
	_name.reset();
	if (_bfr)
		_bfr->reset();

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

void DNSQuery::ntohs()
{
	_id	= ::ntohs(_id);
	_flag	= ::ntohs(_flag);
	_n_qry	= ::ntohs(_n_qry);
	_n_ans	= ::ntohs(_n_ans);
	_n_aut	= ::ntohs(_n_aut);
	_n_add	= ::ntohs(_n_add);
	_type	= ::ntohs(_type);
	_class	= ::ntohs(_class);
}

void DNSQuery::dump()
{
	printf("\n; Buffer\n");
	if (_bfr)
		_bfr->dump_hex();

	printf("; HEADER section\n");
	printf(" id           : %d\n", _id);
	printf(" flag         : %#4X\n", _flag);
	printf(" n question   : %d\n", _n_qry);
	printf(" n answer     : %d\n", _n_ans);
	printf(" n auth       : %d\n", _n_aut);
	printf(" n additional : %d\n", _n_add);
	printf("; QUESTION section\n");
	printf(" type         : %d\n", _type);
	printf(" class        : %d\n", _class);
	printf(" name         : %s\n", _name._v);
	printf("; ANSWER section\n");
	_rr_ans->dump();
	printf("; AUTHENTICATION section\n");
	_rr_aut->dump();
	printf("; ADDITIONAL section\n");
	_rr_add->dump();
}

} /* namespace::vos */
