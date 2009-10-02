/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_OCI_VALUE_HPP
#define	_LIBVOS_OCI_VALUE_HPP	1

#include <oci.h>
#include "Buffer.hpp"

namespace vos {

#define	OCI_T_DFLT_SIZE	4000

enum _oci_type {
	OCI_T_VARCHAR	= 1,
	OCI_T_NUMBER,
	OCI_T_DATE,
	OCI_T_RAW,
	OCI_T_ROWID,
	OCI_T_CHAR,
	OCI_T_BINARY_FLOAT,
	OCI_T_BINARY_DOUBLE,
	OCI_T_TIMESTAMP,
	OCI_T_TIMESTAMP_WITH_TZ,
	OCI_T_TIMESTAMP_WITH_LTZ,
	OCI_T_INTERVAL_Y_TO_M,
	OCI_T_INTERVAL_D_TO_S,
	OCI_T_UROWID
};

class OCIValue : public Buffer {
public:
	static OCIValue * NUMBER(const int pos) {
			return new OCIValue(pos, OCI_T_NUMBER, 22);
	}
	static OCIValue * DATE(const int pos) {
			return new OCIValue(pos, OCI_T_DATE, 8);
	}
	static OCIValue * RAW(const int pos) {
			return new OCIValue(pos, OCI_T_RAW, 2001);
	}
	static OCIValue * ROWID(const int pos) {
			return new OCIValue(pos, OCI_T_ROWID, 11);
	}
	static OCIValue * CHAR(const int pos) {
			return new OCIValue(pos, OCI_T_CHAR, 2001);
	}
	static OCIValue * BINARY_FLOAT(const int pos) {
			return new OCIValue(pos, OCI_T_BINARY_FLOAT, 5);
	}
	static OCIValue * BINARY_DOUBLE(const int pos) {
			return new OCIValue(pos, OCI_T_BINARY_DOUBLE, 9);
	}
	static OCIValue * TIMESTAMP(const int pos) {
			return new OCIValue(pos, OCI_T_TIMESTAMP, 12);
	}
	static OCIValue * TIMESTAMP_WITH_TZ(const int pos) {
			return new OCIValue(pos, OCI_T_TIMESTAMP_WITH_TZ, 14);
	}
	static OCIValue * TIMESTAMP_WITH_LTZ(const int pos) {
			return new OCIValue(pos, OCI_T_TIMESTAMP_WITH_LTZ, 12);
	}
	static OCIValue * INTERVAL_Y_TO_M(const int pos) {
			return new OCIValue(pos, OCI_T_INTERVAL_Y_TO_M, 6);
	}
	static OCIValue * INTERVAL_D_TO_S(const int pos) {
			return new OCIValue(pos, OCI_T_INTERVAL_D_TO_S, 12);
	}
	static OCIValue * UROWID(const int pos) {
			return new OCIValue(pos, OCI_T_UROWID, 3951);
	}

	OCIValue(const int pos, const int type = OCI_T_VARCHAR,
			const int len = OCI_T_DFLT_SIZE);

	int		_t;
	int		_p;
	OCIBind		*_bind;
	OCIDefine	*_define;
private:
	DISALLOW_COPY_AND_ASSIGN(OCIValue);
};

} /* namespace::vos */

#endif
