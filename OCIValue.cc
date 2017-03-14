//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "OCIValue.hh"

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
	if (resize(len)) {
		return -1;
	}
	reset();
	return 0;
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
	OCIValue	*o = new OCIValue();

	if (o) {
		int s = o->init(pos, type, len);
		if (s < 0) {
			delete o;
			o = NULL;
		}
	}
	return o;
}

const char* OCIValue::v()
{
	if (_t == OCI_T_VARCHAR || _t == OCI_T_RAW) {
		_v[_l] = 0;
		_l--;
		_v[_l] = 0;
	}
	return _v;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
