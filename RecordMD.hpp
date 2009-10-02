/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_RECORD_METADATA_HPP
#define	_LIBVOS_RECORD_METADATA_HPP	1

#include "Buffer.hpp"

namespace vos {

class RecordMD {
public:
	RecordMD();
	~RecordMD();

	void dump();
	static void ADD(RecordMD **rmd, RecordMD *md);
	static RecordMD *INIT(const char *meta);

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
	Buffer		*_name;
	Buffer		*_date_format;
	Buffer		*_fltr_v;
	RecordMD	*_next;
private:
	DISALLOW_COPY_AND_ASSIGN(RecordMD);
};

}

#endif
