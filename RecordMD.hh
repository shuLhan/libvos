//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_RECORD_METADATA_HH
#define _LIBVOS_RECORD_METADATA_HH 1

#include "File.hh"
#include "List.hh"

namespace vos {

enum _rmd_type {
	RMD_T_STRING	= 0,
	RMD_T_NUMBER,
	RMD_T_DATE,
	RMD_T_BLOB
};

/**
 * @class		: RecordMD
 * @attr		:
 *	- _idx		: index of current meta-data record.
 *	- _flag		: flag of current meta-data record.
 *	- _type		: type of record that this meta-data hold.
 *	- _left_q	: left character used in record data.
 *	- _right_q	: right character used in record data.
 *	- _start_p	: start position of record data.
 *	- _end_p	: end position of record data.
 *	- _sep		: character used as separator between record data.
 *	- _fltr_idx	: index of filter used in record.
 *	- _fltr_rule	: rule of filter that will be used in record.
 *	- _fop		: pointer to filter function.
 *	- _name		: name of record.
 *	- _date_format	: format of date used in data.
 *	- _fltr_v	: value of filter, if filter is in comparable mode.
 *	- BLOB_SIZE	: static, size of blob header.
 * @desc		:
 *	Record meta-data hold each definition of records in DSV file, or in
 *	general: describing how the record will be parsed later on DSV file.
 *	In example, does the record data started with a single quote, double
 *	quote, or any printable character; what character is used as a
 *	separator between record or does its used fixed position; etc.
 *
 *	It's also defined type of filter that will be applied to record. A
 *	filter is used to accepting or rejecting record after parsed.
 */
class RecordMD : public Object {
public:
	RecordMD();
	~RecordMD();
	const char* chars();

	int		_idx;
	int		_flag;
	int		_type;
	int		_left_q;
	int		_right_q;
	int		_start_p;
	int		_end_p;
	int		_sep;
	int		_fltr_idx;
	int		_fltr_rule;
	int		(*_fop)(const int, const void *, const void *);
	Buffer		_name;
	Buffer*		_date_format;
	Buffer*		_fltr_v;

	static List* INIT(const char* meta);
	static List* INIT_FROM_FILE(const char* fmeta);

	static int	BLOB_SIZE;
	static int	DEF_SEP;
	static const char* __cname;
private:
	RecordMD(const RecordMD&);
	void operator=(const RecordMD&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
