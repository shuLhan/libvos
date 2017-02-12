/**
 * Copyright 2017 M. Shulhan (ms@kilabit.info). All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _LIBVOS_DNS_RECORD_TYPE_HH
#define _LIBVOS_DNS_RECORD_TYPE_HH 1

#include "Object.hh"

namespace vos {

class DNSRecordType : public Object {
public:
	DNSRecordType();
	~DNSRecordType();

	static const int SIZE;
	static const char* NAMES[];
	static const int VALUES[];

	static const char* GET_NAME(const int type);
	static int GET_VALUE(const char* name);
private:
	DNSRecordType(const DNSRecordType&);
	void operator=(const DNSRecordType&);
};

}

#endif
