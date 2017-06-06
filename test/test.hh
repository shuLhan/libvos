//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//

#include <assert.h>

#include "../Test.hh"

using vos::Test;
using vos::Error;

#define CB(s)		"{ " s " }"
#define SB(s)		"[ " s " ]"

#define SEP_KV		": "
#define SEP_ITEM	","
#define SEP_LINE	"\n"

#define K(k)		"\"" #k "\""
#define V_STR(v)	"\"" v "\""

#define STR_TEST_0	"test 0"
#define STR_TEST_1	"test 1"
#define STR_TEST_2	"test 2"

extern int expectString(const char* exp, const char* got, int to);

// vi: ts=8 sw=8 tw=80:
