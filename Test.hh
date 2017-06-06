//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_TEST_HH
#define _LIBVOS_TEST_HH 1

#include <assert.h>

#include "Dlogger.hh"

namespace vos {

enum EQUALITY {
	IS_LESS_THAN     = -1
,	IS_EQUAL         = 0
,	IS_GREATER_THAN  = 1
,	IS_NOT_EQUAL     = 1
};

class Test : public Dlogger {
public:
	explicit Test(const char* header);
	~Test();

	void start(const char* suffix, const char* msg = NULL);
	void ok();

	int expect_mem(const void *exp, const void *got, size_t len
		, enum EQUALITY e = IS_EQUAL);
	int expect_ptr(const void *exp, const void *got
		, enum EQUALITY e = IS_EQUAL);
	int expect_string(const char* exp, const char* got
		, enum EQUALITY e = IS_EQUAL);
	int expect_signed(const ssize_t exp, const ssize_t got
		, enum EQUALITY e = IS_EQUAL);
	int expect_unsigned(const size_t exp, const size_t got
		, enum EQUALITY e = IS_EQUAL);
	int expect_double(const double exp, const double got
		, enum EQUALITY e = IS_EQUAL);

protected:
	Buffer _header;
	Buffer _prefix;

private:
	Test(const Test&);
	void operator=(const Test&);
};

}

#endif

// vi: ts=8 sw=8 tw=80:
