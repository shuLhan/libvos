//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../Config.hh"

using vos::Config;

Test T("Config");

const char* save_as = "CONFIG.set";

void test_load()
{
	T.start("load()");

	Config cfg;

	Error err = cfg.load("CONFIG");

	T.expect_error(NULL, err);

	struct {
		const char*    exp_head;
		const char*    exp_key;
		const char*    exp_value;
		const long int exp_num;
	} const tests[] = {{
		"head01"
	,	"key"
	,	"v1"
	,	0
	},{
		"head01"
	,	"key2"
	,	NULL
	,	1234
	},{
		"head02"
	,	"key"
	,	NULL
	,	0
	},{
		"head03"
	,	"key1"
	,	"v3"
	,	0
	},{
		"head03"
	,	"key2"
	,	"# empty"
	,	0
	}};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {

		if (tests[x].exp_value) {
			const char* got = cfg.get(tests[x].exp_head, tests[x].exp_key);

			T.expect_string(tests[x].exp_value, got);
		} else if (tests[x].exp_num) {
			long int got = cfg.get_number(tests[x].exp_head
				, tests[x].exp_key);

			T.expect_signed(tests[x].exp_num, got);
		}
	}

	T.ok();
}

void test_get()
{
	T.start("get()");

	Config cfg;

	Error err = cfg.load(save_as);

	T.expect_error(NULL, err);

	struct {
		const char*    exp_head;
		const char*    exp_key;
		const char*    exp_value;
		const long int exp_num;
	} const tests[] = {{
		"head01"
	,	"key"
	,	"v3"
	,	0
	},{
		"head01"
	,	"key2"
	,	NULL
	,	12345
	},{
		"head02"
	,	"key"
	,	NULL
	,	0
	},{
		"head02"
	,	"key2"
	,	"v2"
	,	0
	},{
		"head03"
	,	"key1"
	,	"v3"
	,	0
	},{
		"head03"
	,	"key2"
	,	"# empty"
	,	0
	},{
		"head04"
	,	"key"
	,	"value"
	,	0
	},{
		"head04"
	,	"key2"
	,	NULL
	,	0
	}};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {

		if (tests[x].exp_value) {
			const char* got = cfg.get(tests[x].exp_head, tests[x].exp_key);

			T.expect_string(tests[x].exp_value, got);
		} else if (tests[x].exp_num) {
			long int got = cfg.get_number(tests[x].exp_head
				, tests[x].exp_key);

			T.expect_signed(tests[x].exp_num, got);
		}
	}

	unlink(save_as);

	T.ok();
}

void test_set()
{
	T.start("set()");

	Config cfg;

	Error err = cfg.load("CONFIG");

	T.expect_error(NULL, err);

	struct {
		const char*    exp_head;
		const char*    exp_key;
		const char*    exp_value;
		const int      exp_ret;
	} const tests[] = {{
		"head01"
	,	"key"
	,	"v3"
	,	0
	},{
		"head01"
	,	"key2"
	,	"12345"
	,	0
	},{
		"head02"
	,	"key2"
	,	"v2"
	,	1
	},{
		"head04"
	,	"key"
	,	"value"
	,	1
	}};

	const size_t tests_len = ARRAY_SIZE(tests);

	for (size_t x = 0; x < tests_len; x++) {
		int got =  cfg.set(tests[x].exp_head, tests[x].exp_key
			, tests[x].exp_value);

		T.expect_signed(tests[x].exp_ret, got);
	}

	cfg.add_comment("## This is a comment");

	cfg.save(save_as, vos::CONFIG_SAVE_WITH_COMMENT);

	cfg.close();

	T.ok();
}

int main()
{
	test_load();
	test_set();
	test_get();

	return 0;
}
