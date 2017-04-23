//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Buffer.test.hh"

Test T("Buffer");

void test_constructor()
{
	T.start("constructor", "default");

	Buffer a;

	T.expect_string(a.__cname, "Buffer");
	T.expect_string(a.Object::__cname, "Object");

	assert(a.size() == Buffer::DFLT_SIZE);
	assert(a.len() == 0);
	T.expect_string(a.v(), "");

	T.ok();
}

void test_constructor_size()
{
	T.start("constructor", "with size");

	Buffer b(256);

	assert(b.len() == 0);
	assert(b.size() == 256);

	T.expect_string(b.v(), "");

	T.ok();
}

void test_constructor_raw()
{
	T.start("constructor", "with raw char");

	const char* exp = "Use of this source code is governed by a BSD-style";
	size_t exp_len = strlen(exp);

	Buffer b(exp);

	assert(b.len() == exp_len);
	assert(b.size() == exp_len);
	T.expect_string(b.v(), exp);

	T.ok();

	T.start(0, "with raw char and size");

	Buffer c(exp, 6);

	assert(c.len() == 6);
	assert(c.size() == 6);
	T.expect_string(c.v(), "Use of");

	T.ok();
}

void test_constructor_buffer()
{
	T.start("constructor", "with empty buffer");

	const char* exp = "Use of this source code is governed by a BSD-style";
	size_t exp_len = strlen(exp);

	Buffer a(256);
	Buffer b(&a);

	assert(b.len() == 0);
	assert(b.size() == a.size());
	T.expect_string(a.v(), b.v());

	T.ok();

	T.start(0, "with non-empty buffer");

	a.copy_raw(exp);

	Buffer c(&a);

	assert(c.len() == exp_len);
	assert(c.size() == exp_len);
	T.expect_string(c.v(), exp);

	T.ok();
}

void test_detach()
{
	T.start("detach");

	Buffer a;
	char *v;

	v = a.detach();

	assert(a.len() == 0);
	assert(a.size() == 0);
	assert(a.v() == NULL);
	assert(v != NULL);

	free(v);

	T.ok();
}

void test_release()
{
	T.start("release");

	Buffer a;

	assert(a.len() == 0);
	assert(a.size() == Buffer::DFLT_SIZE);
	assert(a.v() != NULL);

	a.release();

	assert(a.len() == 0);
	assert(a.size() == 0);
	assert(a.v() == NULL);

	T.ok();
}

void test_reset()
{
	T.start("reset");

	const char* exp = "a string";
	const size_t exp_len = strlen(exp);
	Buffer a;

	a.copy_raw(exp);

	assert(a.len() == exp_len);
	assert(a.size() == Buffer::DFLT_SIZE);
	T.expect_string(exp, a.v(), 0);

	a.reset();

	assert(a.len() == 0);
	assert(a.size() == Buffer::DFLT_SIZE);
	T.expect_string("", a.v(), 0);

	T.ok();
}

void test_trim()
{
	T.start("trim");

	const size_t in_len = 3;
	const char* in[in_len] = {
			"	  a"
		,	"a	  "
		,	"	  a	  "
		};
	const char* exp = "a";

	Buffer a;

	for (size_t x = 0; x < in_len; x++) {
		a.copy_raw(in[x]);
		a.trim();
		T.expect_string(exp, a.v(), 0);
	}

	T.ok();
}

void test_truncate()
{
	T.start("truncate");

	const char* input = "abcdefghijklmnopqrstuvwxyz";
	const char* exp = "abcd";
	const size_t input_len = strlen(input);
	const size_t exp_len = strlen(exp);

	Buffer a;

	a.copy_raw(input);

	assert(a.len() == input_len);
	assert(a.size() == input_len);

	a.truncate(3);

	assert(a.len() == exp_len);
	assert(a.size() == input_len);

	T.expect_string(exp, a.v(), 0);

	T.ok();
}

void test_is_empty()
{
	T.start("is_empty", "true");

	Buffer a;

	assert(a.is_empty() == 1);

	T.ok();

	T.start("is_empty", "false");

	a.copy_raw("any");

	assert(a.is_empty() == 0);

	T.ok();
}

void test_set_len()
{
	T.start("set_len", "it should not resize the buffer");

	Buffer b;

	assert(b.len() == 0);
	assert(b.size() == Buffer::DFLT_SIZE);

	b.set_len(b.size());

	assert(b.len() == b.size());
	assert(b.size() == Buffer::DFLT_SIZE);

	T.ok();

	T.start(0, "it should resize the buffer");

	size_t new_len = 24;

	b.set_len(new_len);

	assert(b.len() == new_len);
	assert(b.size() == (new_len + 1));

	T.ok();
}

void test_resize()
{
	T.start("resize", "greater than current size");

	size_t resize_to = 256;
	const char* exp = "buffer";
	const size_t exp_len = strlen(exp);
	Buffer a;

	a.copy_raw(exp);

	assert(a.len() == exp_len);
	assert(a.size() == Buffer::DFLT_SIZE);
	T.expect_string(exp, a.v(), 0);

	a.resize(resize_to);

	assert(a.len() == exp_len);
	assert(a.size() == resize_to);
	T.expect_string(exp, a.v(), 0);

	T.ok();

	T.start(0, "less than current size");

	const size_t resize_to_less = 16;

	a.resize(resize_to_less);

	assert(a.len() == exp_len);
	assert(a.size() == resize_to);
	T.expect_string(exp, a.v(), 0);

	T.ok();
}

void test_char_at()
{
	const struct t_char_at inputs[] = {
		{
			"With empty buffer",
			"",
			0,
			0,
		},
		{
			"With index out of range",
			"Test",
			100,
			0,
		},
		{
			"With index same as input length",
			"Test",
			4,
			0,
		},
		{
			"With correct index",
			"Test",
			0,
			'T',
		},
		{
			"With correct index",
			"Test",
			3,
			't',
		},
	};

	Buffer b;
	const size_t inputs_len = ARRAY_SIZE(inputs);

	for (size_t x = 0; x < inputs_len; x++) {
		T.start("char_at", inputs[x].desc);

		b.copy_raw(inputs[x].in);

		char got = b.char_at(inputs[x].idx);

		assert(inputs[x].exp == got);

		T.ok();
	}
}

void test_set_char_at()
{
	const struct t_set_char_at inputs[] = {
		{
			"With empty input",
			"",
			0,
			'x',
			-1,
			"",
		},
		{
			"With out of range index",
			"Test",
			10,
			'x',
			-1,
			"Test",
		},
		{
			"With index equal to length",
			"Test",
			4,
			'x',
			-1,
			"Test",
		},
		{
			"Within range",
			"Test",
			3,
			'x',
			0,
			"Tesx",
		},
	};

	Buffer b;
	size_t inputs_len = ARRAY_SIZE(inputs);

	for (size_t x = 0; x < inputs_len; x++) {
		T.start("set_char_at", inputs[x].desc);

		b.copy_raw(inputs[x].in);

		int got_ret = b.set_char_at(inputs[x].idx, inputs[x].v);

		assert(inputs[x].exp_ret == got_ret);

		T.expect_string(inputs[x].exp_res, b.v(), 0);

		T.ok();
	}
}

void test_copy()
{
	const struct t_copy inputs[] = {
		{
			"With null",
			NULL,
			0,
			Buffer::DFLT_SIZE,
			"",
		},
		{
			"With buffer greater than default size",
			new Buffer(TEST_IN_MED),
			test_in_med_len,
			test_in_med_len,
			TEST_IN_MED,
		},
	};

	Buffer b;
	size_t inputs_len = ARRAY_SIZE(inputs);

	for (size_t x = 0; x< inputs_len; x++) {
		T.start("copy()", inputs[x].desc);

		b.copy(inputs[x].in);

		assert(inputs[x].exp_len == b.len());
		assert(inputs[x].exp_size == b.size());
		T.expect_string(inputs[x].exp_v, b.v(), 0);

		if (inputs[x].in) {
			delete inputs[x].in;
		}

		T.ok();
	}
}

void test_copy_raw()
{
	const struct t_copy_raw inputs[] = {
		{
			"With empty string",
			NULL,
			0,
			0,
			Buffer::DFLT_SIZE,
			"",
		},
		{
			"With short string",
			TEST_IN_SHORT,
			0,
			test_in_short_len,
			Buffer::DFLT_SIZE,
			TEST_IN_SHORT,
		},
		{
			"With medium string",
			TEST_IN_MED,
			0,
			test_in_med_len,
			test_in_med_len,
			TEST_IN_MED,
		},
		{
			"With length",
			TEST_IN_MED,
			3,
			3,
			Buffer::DFLT_SIZE,
			"Use",
		},
	};

	size_t inputs_len = ARRAY_SIZE(inputs);

	for (size_t x = 0; x < inputs_len; x++) {
		T.start("copy_raw()", inputs[x].desc);

		Buffer b;

		b.copy_raw(inputs[x].in, inputs[x].in_len);

		assert(inputs[x].exp_len == b.len());
		assert(inputs[x].exp_size == b.size());
		T.expect_string(inputs[x].exp_v, b.v(), 0);

		T.ok();
	}
}

Buffer in;
List* lbuf;

void test_split_by_char_n(const char* input, const char split
	, const uint8_t trim, const char* exp, const int exp_size)
{
	in.copy_raw(input);
	lbuf = in.split_by_char(split, trim);

	expectString(exp, lbuf->chars(), 0);

	assert(lbuf->size() == exp_size);
	expectString(exp, lbuf->chars(), 0);
	delete lbuf;
}

void test_split_by_char()
{
	in.reset();
	lbuf = in.split_by_char(',');
	assert(lbuf == NULL);

	test_split_by_char_n(TEST_SPLIT_BY_00_IN, ',', 0
		, TEST_SPLIT_BY_00_OUT, 1);

	test_split_by_char_n(TEST_SPLIT_BY_01_IN, ':', 0
		, TEST_SPLIT_BY_01_OUT, 2);

	test_split_by_char_n(TEST_SPLIT_BY_02_IN, ',', 0
		, TEST_SPLIT_BY_02_OUT, 4);

	test_split_by_char_n(TEST_SPLIT_BY_02_IN, ',', 1
		, TEST_SPLIT_BY_02_OUT_TRIM, 3);

	test_split_by_char_n(TEST_SPLIT_BY_03_IN, ',', 0
		, TEST_SPLIT_BY_03_OUT, 6);

	test_split_by_char_n(TEST_SPLIT_BY_03_IN, ',', 1
		, TEST_SPLIT_BY_03_OUT_TRIM, 3);
}

void test_split_by_whitespace_n(const char* input
	, const char* exp, const int exp_size)
{
	in.copy_raw(input);
	lbuf = in.split_by_whitespace();

	expectString(exp, lbuf->chars(), 0);

	assert(lbuf->size() == exp_size);
	expectString(exp, lbuf->chars(), 0);
	delete lbuf;
}

void test_split_by_whitespace()
{
	in.reset();

	lbuf = in.split_by_whitespace();
	assert(lbuf == NULL);

	test_split_by_whitespace_n(TEST_SPLIT_BY_00_IN
		, TEST_SPLIT_BY_WS_00_OUT, 1);
	test_split_by_whitespace_n(TEST_SPLIT_BY_02_IN
		, TEST_SPLIT_BY_WS_02_OUT, 7);
	test_split_by_whitespace_n(TEST_SPLIT_BY_03_IN
		, TEST_SPLIT_BY_WS_03_OUT, 8);
	test_split_by_whitespace_n(TEST_04_IN, TEST_04_WS_OUT, 5);
}

void test_PARSE_INT()
{
	int s = 0;
	int v = 0;
	char* str = (char*) calloc(64, sizeof(char));
	char* p;

	strcpy(str, "\0");
	p = str;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 0);
	assert(*p == 0);

	strcpy(str, "-asdf");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 0);
	assert(*p == '-');

	strcpy(str, "0");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 0);
	assert(*p == 0);

	strcpy(str, "0123456");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 123456);
	assert(*p == 0);

	strcpy(str, "-0123456");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == -123456);
	assert(*p == 0);

	strcpy(str, "0123as");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == 123);
	assert(*p == 'a');

	strcpy(str, "-0123as");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == 0);
	assert(v == -123);
	assert(*p == 'a');

	// overflow
	strcpy(str, "9876543210as");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == -1);
	assert(v == 0);
	assert(*p == '9');

	// underflow
	strcpy(str, "-9876543210as");
	p = str;
	v = 0;
	s = Buffer::PARSE_INT(&p, &v);
	assert(s == -1);
	assert(v == 0);
	assert(*p == '-');

	free(str);
}

void test_aprint()
{
	const char* exps[] = {
			"%11209876543210"
		,	"%---##++"
		,	"%---#1"
		,	"%-.11209876543210"
		,	"112.987654"
		,	"  3"
		,	"113.0"
		,	"112.988"
		};

	Buffer b;
	double d = 112.987654321;
	int exp_idx = 0;

	b.reset();
	b.aprint("%11209876543210", d);
	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%---##++", d);
	expectString(exps[exp_idx++], b.chars(), 0);


	b.reset();
	b.aprint("%---#1", d);
	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%-.11209876543210", d);
	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%f", d);

	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%3d", 3);

	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%.1f", d);

	expectString(exps[exp_idx++], b.chars(), 0);

	b.reset();
	b.aprint("%.3f", d);

	expectString(exps[exp_idx++], b.chars(), 0);
}

int main()
{
	test_constructor();
	test_constructor_size();
	test_constructor_raw();
	test_constructor_buffer();

	test_detach();
	test_release();
	test_reset();
	test_trim();
	test_truncate();

	test_is_empty();

	// skip testing `len()`, because its already done on other tests.
	test_set_len();

	// skip testing `size()`, because its already done on other tests.
	test_resize();

	// skip testing `v()`, because its already done on other tests.

	test_char_at();
	test_set_char_at();

	test_copy();
	test_copy_raw();

	test_split_by_char();

	test_split_by_whitespace();

	test_PARSE_INT();

	//test_aprint();

	return 0;
}

// vi: ts=8 sw=8 tw=78:
