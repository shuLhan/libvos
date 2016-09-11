//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_OCI_VALUE_HH
#define _LIBVOS_OCI_VALUE_HH 1

#include <oci.h>
#include "Buffer.hh"

namespace vos {

enum _oci_type {
	OCI_T_VARCHAR	= 1
,	OCI_T_NUMBER
,	OCI_T_DATE
,	OCI_T_RAW
,	OCI_T_ROWID
,	OCI_T_CHAR
,	OCI_T_BINARY_FLOAT
,	OCI_T_BINARY_DOUBLE
,	OCI_T_TIMESTAMP
,	OCI_T_TIMESTAMP_WITH_TZ
,	OCI_T_TIMESTAMP_WITH_LTZ
,	OCI_T_INTERVAL_Y_TO_M
,	OCI_T_INTERVAL_D_TO_S
,	OCI_T_UROWID
};

/**
 * @class		: OCIValue
 * @attr		:
 *	- _t		: type of value data.
 *	- _p		: position of value on SQL query.
 *	- _bind		: OCIBind object.
 *	- _define	: OCIDefine object.
 *	- DFLT_SIZE	: static, default OCIValue buffer size.
 * @desc		:
 *	module for handling OCI result set value.
 */
class OCIValue : public Buffer {
public:
	OCIValue();
	~OCIValue();

	int init(const int pos, const int type = OCI_T_VARCHAR
		, const int len = DFLT_SIZE);

	static OCIValue* INIT(const int pos, const int type = OCI_T_VARCHAR
				, const int len = DFLT_SIZE);

	static inline OCIValue * NUMBER(const int pos) {
		return OCIValue::INIT(pos, OCI_T_NUMBER, 22);
	}
	static inline OCIValue * DATE(const int pos) {
		return OCIValue::INIT(pos, OCI_T_DATE, 8);
	}
	static inline OCIValue * RAW(const int pos) {
		return OCIValue::INIT(pos, OCI_T_RAW, 2001);
	}
	static inline OCIValue * ROWID(const int pos) {
		return OCIValue::INIT(pos, OCI_T_ROWID, 11);
	}
	static inline OCIValue * CHAR(const int pos) {
		return OCIValue::INIT(pos, OCI_T_CHAR, 2001);
	}
	static inline OCIValue * BINARY_FLOAT(const int pos) {
		return OCIValue::INIT(pos, OCI_T_BINARY_FLOAT, 5);
	}
	static inline OCIValue * BINARY_DOUBLE(const int pos) {
		return OCIValue::INIT(pos, OCI_T_BINARY_DOUBLE, 9);
	}
	static inline OCIValue * TIMESTAMP(const int pos) {
		return OCIValue::INIT(pos, OCI_T_TIMESTAMP, 12);
	}
	static inline OCIValue * TIMESTAMP_WITH_TZ(const int pos) {
		return OCIValue::INIT(pos, OCI_T_TIMESTAMP_WITH_TZ, 14);
	}
	static inline OCIValue * TIMESTAMP_WITH_LTZ(const int pos) {
		return OCIValue::INIT(pos, OCI_T_TIMESTAMP_WITH_LTZ, 12);
	}
	static inline OCIValue * INTERVAL_Y_TO_M(const int pos) {
		return OCIValue::INIT(pos, OCI_T_INTERVAL_Y_TO_M, 6);
	}
	static inline OCIValue * INTERVAL_D_TO_S(const int pos) {
		return OCIValue::INIT(pos, OCI_T_INTERVAL_D_TO_S, 12);
	}
	static inline OCIValue * UROWID(const int pos) {
		return OCIValue::INIT(pos, OCI_T_UROWID, 3951);
	}

	const char* v();

	int		_t;
	int		_p;
	OCIBind		*_bind;
	OCIDefine	*_define;

	static unsigned int DFLT_SIZE;
private:
	OCIValue(const OCIValue&);
	void operator=(const OCIValue&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
