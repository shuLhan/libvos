/**
 * Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DNSRecordType.hh"

namespace vos {

const char* DNSRecordType::__cname = "DNSRecordType";

const int DNSRecordType::SIZE = 22;

const char* DNSRecordType::NAMES[DNSRecordType::SIZE] = {
		"A"	,"NS"	,"MD"	,"MF",	"CNAME"
	,	"SOA"	,"MB"	,"MG"	,"MR",	"NULL"
	,	"WKS"	,"PTR"	,"HINFO","MINFO","MX"
	,	"TXT"	,"AAAA"	,"SRV"	,"AXFR"	,"MAILB"
	,	"MAILA"	,"*"
	};

const int DNSRecordType::VALUES[DNSRecordType::SIZE] = {
		1	,2	,3	,4	,5
	,	6	,7	,8	,9	,10
	,	11	,12	,13	,14	,15
	,	16	,28	,33	,252	,253
	,	254	,255
	};

DNSRecordType::DNSRecordType() : Object()
{}

DNSRecordType::~DNSRecordType()
{}

/**
 * `GET_NAME()` will search list of record type with value is `type` and
 * return their NAME representation.
 *
 * It will return NULL if no match found.
 */
const char* DNSRecordType::GET_NAME(const int type)
{
	int x = 0;

	for (; x < SIZE; x++) {
		if (VALUES[x] == type) {
			return NAMES[x];
		}
	}

	return NULL;
}

/**
 * `GET_VALUE()` will search list of record type that match with `name` and
 * return their TYPE representation.
 *
 * It will return -1 if no match found.
 */
int DNSRecordType::GET_VALUE(const char* name)
{
	int x = 0;

	for (; x < SIZE; x++) {
		if (strcasecmp(NAMES[x], name) == 0) {
			return VALUES[x];
		}
	}

	return -1;
}

}
