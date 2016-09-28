//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hh"
#include "../SSVReader.hh"

#define	EXP_LINE_00	SB(V_STR("#"))
#define	EXP_LINE_01	SB( \
		V_STR("#") SEP_ITEM \
		V_STR("/etc/hosts:") SEP_ITEM \
		V_STR("static") SEP_ITEM \
		V_STR("lookup") SEP_ITEM \
		V_STR("table") SEP_ITEM \
		V_STR("for") SEP_ITEM \
		V_STR("host") SEP_ITEM \
		V_STR("names") \
	)
#define	EXP_LINE_02	SB(V_STR("#"))
#define EXP_LINE_03	SB( \
		V_STR("#<ip-address>") SEP_ITEM \
		V_STR("<hostname.domain.org>") SEP_ITEM \
		V_STR("<hostname>") \
	)
#define	EXP_LINE_04	SB( \
		V_STR("127.0.0.1") SEP_ITEM \
		V_STR("localhost.localdomain") SEP_ITEM \
		V_STR("localhost") \
	)
#define	EXP_LINE_05	SB( \
		V_STR("::1") SEP_ITEM \
		V_STR("localhost.localdomain") SEP_ITEM \
		V_STR("localhost") \
	)
#define	EXP_LINE_06	SB( \
		V_STR("127.0.0.1") SEP_ITEM \
		V_STR("bubu") \
	)
#define	EXP_LINE_07	SB( \
		V_STR("#") SEP_ITEM \
		V_STR("jquery-wp-content") \
	)
#define	EXP_LINE_08	SB( \
		V_STR("127.0.0.1") SEP_ITEM \
		V_STR("local.jquery.com") SEP_ITEM \
		V_STR("local.api.jquery.com") SEP_ITEM \
		V_STR("local.blog.jquery.com") \
	)
#define	EXP_LINE_09	SB( \
		V_STR("#") SEP_ITEM \
		V_STR("End") SEP_ITEM \
		V_STR("of") SEP_ITEM \
		V_STR("file") \
	)

int main (int argc, char** argv)
{
	register int s;
	vos::SSVReader reader;
	const char* got = NULL;

	s = reader.load ("./hosts");

	assert(s == 0);

	assert(reader._rows->size() == 10);

	got = reader._rows->at(0)->chars();
	assert(strcmp(EXP_LINE_00, got) == 0);

	got = reader._rows->at(1)->chars();
	assert(strcmp(EXP_LINE_01, got) == 0);

	got = reader._rows->at(2)->chars();
	assert(strcmp(EXP_LINE_02, got) == 0);

	got = reader._rows->at(3)->chars();
	assert(strcmp(EXP_LINE_03, got) == 0);

	got = reader._rows->at(4)->chars();
	assert(strcmp(EXP_LINE_04, got) == 0);

	got = reader._rows->at(5)->chars();
	assert(strcmp(EXP_LINE_05, got) == 0);

	got = reader._rows->at(6)->chars();
	assert(strcmp(EXP_LINE_06, got) == 0);

	got = reader._rows->at(7)->chars();
	assert(strcmp(EXP_LINE_07, got) == 0);

	got = reader._rows->at(8)->chars();
	assert(strcmp(EXP_LINE_08, got) == 0);

	got = reader._rows->at(9)->chars();
	assert(strcmp(EXP_LINE_09, got) == 0);

	// circular, back to 0 element.
	got = reader._rows->at(10)->chars();
	assert(strcmp(EXP_LINE_00, got) == 0);

	reader.reset ();
	reader._comment_c = '#';

	s = reader.load ("./hosts");

	assert(s == 0);
	assert(reader._rows->size() == 4);

	got = reader._rows->at(0)->chars();
	assert(strcmp(EXP_LINE_04, got) == 0);

	got = reader._rows->at(1)->chars();
	assert(strcmp(EXP_LINE_05, got) == 0);

	got = reader._rows->at(2)->chars();
	assert(strcmp(EXP_LINE_06, got) == 0);

	got = reader._rows->at(3)->chars();
	assert(strcmp(EXP_LINE_08, got) == 0);

	// circular, back to 0 element.
	got = reader._rows->at(4)->chars();
	assert(strcmp(EXP_LINE_04, got) == 0);

	return s;
}

// vi: ts=8 sw=8 tw=78:
