/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "OCIValue.hpp"

namespace vos {

unsigned int OCIValue::DFLT_SIZE = 4000;

/**
 * @method	: OCIValue::OCIValue
 * @desc	: OCIValue object constructor.
 */
OCIValue::OCIValue() : Buffer()
,	_t(0)
,	_p(0)
,	_bind(NULL)
,	_define(NULL)
{}

OCIValue::~OCIValue()
{
	if (_define) {
		OCIHandleFree(_define, OCI_HTYPE_DEFINE);
		_define = NULL;
	}
	if (_bind) {
		OCIHandleFree(_bind, OCI_HTYPE_BIND);
		_bind = NULL;
	}
}

/**
 * @method	: OCIValue::init
 * @param	:
 *	> pos	: position of this value in SQL query.
 *	> type	: type of value data.
 *	> len	: initial size of buffer.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: initialize OCIValue object.
 */
int OCIValue::init(const int pos, const int type, const int len)
{
	_p = pos;
	_t = type;
	reset();
	return resize(len);
}

/**
 * @method		: OCIValue::INIT
 * @param		:
 *	> pos		: position of this value in SQL query.
 *	> type		: type of value data.
 *	> len		: initial size of buffer.
 * @return		:
 *	< OCIValue	: success, pointer to a new OCIValue object.
 *	< NULL		: fail.
 * @desc		: create and initialize a new OCIValue object.
 */
OCIValue* OCIValue::INIT(const int pos, const int type, const int len)
{
	register int	s;
	OCIValue	*o = new OCIValue();

	if (o) {
		s = o->init(pos, type, len);
		if (s < 0) {
			delete o;
			o = NULL;
		}
	}
	return o;
}

} /* namespace::vos */
