/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Reader.hpp"

namespace vos {

Reader::Reader()
{}

Reader::~Reader()
{}

/**
 * @desc: read one row from file, using 'r' as record buffer, and 'md' as
 * record meta data.
 *
 * @param:
 *	> r	: record buffer, already allocated by user.
 *	> md	: record meta data, already set by user.
 *	
 * @return:
 *	< 1	: one record readed.
 *	< 0	: EOF.
 *	< -1	: record rejected.
 */
int Reader::read(Record *r, RecordMD *rmd)
{
	int	startp	= 0;
	Buffer	*line	= File::get_line();

	if (! line)
		return 0;

	while (rmd && r) {
		/* set start position */
		if (rmd->_start_p) {
			if (startp < rmd->_start_p) {
				startp += (rmd->_start_p - startp);
				if (startp > line->_i)
					goto reject;
			}
			if (rmd->_left_q) {
				if (line->_v[startp + 1] == rmd->_left_q)
					++startp;
			}
		} else if (rmd->_left_q) {
			if (line->_v[startp] != rmd->_left_q)
				goto reject;
			++startp;
		}

		if (rmd->_end_p) {
			while (startp < rmd->_end_p) {
				r->appendc(line->_v[startp]);
				++startp;
				if (startp > line->_i)
					goto try_next_col;
			}
		} else if (rmd->_right_q) {
			while (line->_v[startp] != rmd->_right_q) {
				r->appendc(line->_v[startp]);
				++startp;
				if (startp > line->_i)
					goto try_next_col;
			}

			++startp;
			if (startp > line->_i)
				goto try_next_col;

			if (rmd->_sep) {
				while (line->_v[startp] != rmd->_sep) {
					++startp;
					if (startp > line->_i)
						goto try_next_col;
				}
				++startp;
				if (startp > line->_i)
					goto try_next_col;
			}
		} else if (rmd->_sep) {
			while (line->_v[startp] != rmd->_sep) {
				r->appendc(line->_v[startp]);
				++startp;
				if (startp > line->_i)
					goto try_next_col;
			}
			++startp;
		} else {
			r->append(&line->_v[startp], line->_i - startp);
			startp += r->_i;
		}

try_next_col:
		r	= r->_next_col;
		rmd	= rmd->_next;
	}

	return 1;
reject:
	return -1;
}

}
