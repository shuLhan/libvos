/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "OCIValue.hpp"

namespace vos {

unsigned int OCIValue::DFLT_SIZE = 4000;

OCIValue::OCIValue() : Buffer(),
	_t(0),
	_p(0),
	_bind(NULL),
	_define(NULL)
{}

int OCIValue::init(const int pos, const int type, const int len)
{
	_p = pos;
	_t = type;
	return init_size(len);
}

OCIValue* OCIValue::INIT(const int pos, const int type, const int len)
{
	int		s;
	OCIValue	*o = new OCIValue();

	if (o) {
		s = o->init(pos, type, len);
		if (s) {
			delete o;
			o = NULL;
		}
	}
	return o;
}

} /* namespace::vos */
