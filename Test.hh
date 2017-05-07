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

class Test : public Dlogger {
public:
	explicit Test(const char* header);
	~Test();

	void start(const char* suffix, const char* msg = 0);
	void ok();

	int expect_mem(const void *exp, const void *got, size_t len,
		int equality = 0);
	int expect_ptr(const void *exp, const void *got, int equality = 0);
	int expect_string(const char* exp, const char* got, int equality = 0);
	int expect_signed(const ssize_t exp, const ssize_t got,
		int equality = 0);
	int expect_unsigned(const size_t exp, const size_t got,
		int equality = 0);

protected:
	Buffer _header;
	Buffer _prefix;

private:
	Test(const Test&);
	void operator=(const Test&);
};

}

#endif
