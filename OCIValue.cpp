/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "OCIValue.hpp"

namespace vos {

OCIValue::OCIValue(const int pos, const int type, const int len) : Buffer(len),
	_t(type),
	_p(pos),
	_bind(NULL),
	_define(NULL)
{}

} /* namespace::vos */
