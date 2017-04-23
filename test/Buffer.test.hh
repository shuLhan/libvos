//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../Test.hh"
#include "../List.hh"

using vos::Test;
using vos::List;

#define TEST_SPLIT_BY_00_IN "127.0.0.1:53"
#define TEST_SPLIT_BY_00_OUT SB(V_STR(TEST_SPLIT_BY_00_IN))
#define TEST_SPLIT_BY_WS_00_OUT SB(V_STR(TEST_SPLIT_BY_00_IN))

#define TEST_SPLIT_BY_01_IN "127.0.0.1:53"
#define TEST_SPLIT_BY_01_OUT SB(K(127.0.0.1) SEP_ITEM K(53))

#define TEST_SPLIT_BY_02_IN "a b, c d e, fg hi, "
#define	TEST_SPLIT_BY_WS_02_OUT SB( \
		V_STR("a") SEP_ITEM \
		V_STR("b,") SEP_ITEM \
		V_STR("c") SEP_ITEM \
		V_STR("d") SEP_ITEM \
		V_STR("e,") SEP_ITEM \
		V_STR("fg") SEP_ITEM \
		V_STR("hi,") \
	)

#define TEST_SPLIT_BY_02_OUT SB( \
		K(a b) SEP_ITEM \
		V_STR(" c d e") SEP_ITEM \
		V_STR(" fg hi") SEP_ITEM \
		V_STR(" ") \
	)

#define TEST_SPLIT_BY_02_OUT_TRIM SB( \
		K(a b) SEP_ITEM \
		K(c d e) SEP_ITEM \
		K(fg hi) \
	)

#define TEST_SPLIT_BY_03_IN "a b,, c d e, , fg hi, "
#define TEST_SPLIT_BY_03_OUT SB( \
		V_STR("a b") SEP_ITEM \
		V_STR("") SEP_ITEM \
		V_STR(" c d e") SEP_ITEM \
		V_STR(" ") SEP_ITEM \
		V_STR(" fg hi") SEP_ITEM \
		V_STR(" ") \
	)
#define TEST_SPLIT_BY_03_OUT_TRIM SB( \
		K(a b) SEP_ITEM \
		K(c d e) SEP_ITEM \
		K(fg hi) \
	)
#define TEST_SPLIT_BY_WS_03_OUT SB( \
		V_STR("a") SEP_ITEM \
		V_STR("b,,") SEP_ITEM \
		V_STR("c") SEP_ITEM \
		V_STR("d") SEP_ITEM \
		V_STR("e,") SEP_ITEM \
		V_STR(",") SEP_ITEM \
		V_STR("fg") SEP_ITEM \
		V_STR("hi,") \
	)

#define	TEST_04_IN "	  a   b   c		d	e	"
#define	TEST_04_WS_OUT SB( \
		V_STR("a") SEP_ITEM \
		V_STR("b") SEP_ITEM \
		V_STR("c") SEP_ITEM \
		V_STR("d") SEP_ITEM \
		V_STR("e") \
	)

#define TEST_IN_MED "Use of this source code is governed by a BSD-style"

size_t test_in_med_len = strlen(TEST_IN_MED);

struct t_char_at {
	const char *desc;
	const char *in;
	size_t     idx;
	char       exp;
};

struct t_set_char_at {
	const char *desc;
	const char *in;
	size_t     idx;
	char       v;
	int        exp_ret;
	const char *exp_res;
};

struct t_copy {
	const char   *desc;
	Buffer       *in;
	const size_t exp_len;
	const size_t exp_size;
	const char   *exp_v;
};
