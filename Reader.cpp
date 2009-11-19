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

int Reader::refill_buffer(const int len)
{
	int l;

	l = _i - _p;
	if (l > 0) {
		memmove(_v, &_v[_p], l);

		if (len > _l) {
			resize(len);
			_p += len - _l;
		}

		_i = ::read(_d, &_v[l], _p);
		if (_i < 0) {
			throw Error(E_FILE_READ, _name._v);
		}

		_i += l;
	} else {
		_i = 0;
	}
	_p	= 0;
	_v[_i]	= '\0';

	return _i;
}

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
	int startp	= _p;
	int len		= 0;
	int s		= 0;

	if (_i == 0) {
		s = File::read();
		if (s == 0)
			return 0;
	}

	while (rmd && r) {
		if (rmd->_start_p) {
			if (_p + rmd->_start_p >= _i) {
				s = refill_buffer(rmd->_start_p);
				if (0 == s)
					return 1;
			}

			startp = _p + rmd->_start_p;
		}

		if (rmd->_left_q) {
			if (_v[startp] != rmd->_left_q) {
				goto reject;
			}

			++startp;
			if (startp >= _i) {
				startp = _i - _p;
				s = refill_buffer(0);
				if (0 == s)
					return 1;
			}
		}

		if (rmd->_type == RMD_T_BLOB) {
			len = sizeof(r->_i);
			if (startp + len >= _i) {
				startp = startp - _p;
				s = refill_buffer(0);
				if (0 == s)
					return 1;
			}
			memcpy(&r->_i, &_v[startp], len);
			startp += len;

			r->_i = r->_i - len;

			if (r->_i > r->_l) {
				r->resize(r->_i + 1);
			}

			if (r->_i > _l) {
				startp = startp - _p;
				s = refill_buffer(_l + r->_i);
				if (0 == s)
					return 1;
			}

			if ((startp + r->_i) >= _i) {
				startp = startp - _p;
				s = refill_buffer(r->_i);
				if (0 == s)
					return 1;
			}

			memcpy(r->_v, &_v[startp], r->_i);
			r->_v[r->_i] = '\0';
			startp += r->_i;
			if (startp >= _i) {
				startp = _i - _p;
				s = refill_buffer(0);
				if (0 == s)
					return 1;
			}

			if (rmd->_right_q) {
				if (_v[startp] != rmd->_right_q) {
					goto reject;
				}
				++startp;
				if (startp >= _i) {
					startp = _i - _p;
					s = refill_buffer(0);
					if (0 == s)
						return 1;
				}
			}

			if (rmd->_sep) {
				while (_v[startp] != rmd->_sep) {
					++startp;
					if (startp >= _i) {
						startp = _i - _p;
						s = refill_buffer(0);
						if (0 == s)
							return 1;
					}
				}

				++startp;
				if (startp >= _i) {
					startp = _i - _p;
					s = refill_buffer(0);
					if (0 == s)
						return 1;
				}
			}
		} else { /* not BLOB */
			if (rmd->_end_p) {
				len = (startp - _p) + rmd->_end_p;

				if (len > r->_l) {
					r->resize(len);
				}

				if (startp + len >= _i) {
					startp = startp - _p;
					s = refill_buffer(0);
					if (0 == s)
						return 1;
				}

				memcpy(r->_v, &_v[startp], len);
				startp += len;
			} else if (rmd->_right_q) {
				while (_v[startp] != rmd->_right_q) {
					r->appendc(_v[startp]);
					++startp;
					if (startp >= _i) {
						startp = _i - _p;
						s = refill_buffer(0);
						if (0 == s)
							return 1;
					}
				}

				++startp;
				if (startp >= _i) {
					startp = _i - _p;
					s = refill_buffer(0);
					if (0 == s)
						return 1;
				}

				if (rmd->_sep) {
					while (_v[startp] != rmd->_sep) {
						++startp;
						if (startp >= _i) {
							startp = _i - _p;
							s = refill_buffer(0);
							if (0 == s)
								return 1;
						}
					}

					++startp;
					if (startp >= _i) {
						startp = _i - _p;
						s = refill_buffer(0);
						if (0 == s)
							return 1;
					}
				}
			} else if (rmd->_sep) {
				while (_v[startp] != rmd->_sep) {
					r->appendc(_v[startp]);
					++startp;
					if (startp >= _i) {
						startp = _i - _p;
						s = refill_buffer(0);
						if (0 == s)
							return 1;
					}
				}

				++startp;
				if (startp >= _i) {
					startp = _i - _p;
					s = refill_buffer(0);
					if (0 == s)
						return 1;
				}
			} else {
				while (_v[startp] != __eol[0]) {
					r->appendc(_v[startp]);
					++startp;
					if (startp >= _i) {
						startp = _i - _p;
						s = refill_buffer(0);
						if (0 == s)
							return 1;
					}
				}

				++startp;
				if (startp >= _i) {
					startp = _i - _p;
					s = refill_buffer(0);
					if (0 == s)
						return 1;
				}
			}
		}

		r	= r->_next_col;
		rmd	= rmd->_next;
	}

	while (_v[startp] != __eol[0]) {
		++startp;
		if (startp >= _i) {
			startp = _i - _p;
			s = refill_buffer(0);
			if (0 == s)
				return 1;
		}
	}
	_p = startp + 1;

	return 1;
reject:
	while (_v[startp] != __eol[0]) {
		++startp;
		if (startp >= _i) {
			startp = _i - _p;
			s = refill_buffer(0);
			if (0 == s)
				return 0;
		}
	}
	_p = startp + 1;

	return -1;
}

}
