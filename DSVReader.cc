//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "DSVReader.hh"

namespace vos {

const char* DSVReader::__cname = "DSVReader";

/**
 * @method	: DSVReader::DSVReader
 * @desc	: DSVReader object constructor.
 */
DSVReader::DSVReader()
{}

/**
 * @method	: DSVReader::~DSVReader
 * @desc	: DSVReader object desctructor.
 */
DSVReader::~DSVReader()
{}

/**
 * @method		: DSVReader::refill_buffer
 * @param		:
 *	< read_min	: size to be read from descriptor and fill the buffer.
 * @return		:
 *	> >0		: success, number of bytes filled in buffer.
 *	> 0		: EOF.
 *	> -1		: fail, error at reading descriptor.
 * @desc		:
 *	move unparsed line to the first position, and fill the rest with a new
 *	content.
 */
ssize_t DSVReader::refill_buffer(const size_t read_min)
{
	size_t move_len = 0;
	ssize_t len = 0;
	ssize_t s = 0;

	move_len = _i - _p;
	if (move_len > 0 && _p > ((_i / 2) + 1)) {
		if (LIBVOS_DEBUG) {
			printf("[%s] refill_buffer: memmove from %zu of %zu\n"
				, __cname, _p, _i);
		}
		memmove(&_v[0], &_v[_p], move_len);
	}

	len = ssize_t(move_len + read_min);
	if (size_t(len) > _l) {

		if (LIBVOS_DEBUG) {
			printf("[%s] reader resize: from %zu to %zd\n"
				, __cname, _l, len);
		}

		resize(size_t(len));
		len -= move_len;
	} else {
		len = ssize_t(_l - move_len);
		if (len <= 0) {
			len = ssize_t(_l * 2);

			if (LIBVOS_DEBUG) {
				printf("[%s] reader resize: from %zu to %zd\n"
					, __cname, _l, len);
			}

			resize(size_t(len));
			len -= move_len;
		}
	}

	s = ::read(_d, &_v[move_len], size_t(len));
	if (s < 0) {
		return -1;
	}

	_i = size_t(s);
	_i	+= move_len;
	_p	= 0;
	_v[_i]	= '\0';

	return ssize_t(_i);
}

/**
 * @method	: DSVReader::read
 * @param	:
 *	> r	: return value, record buffer; already allocated by user.
 *	> md	: record meta data, already set by user.
 * @return	:
 *	< 1	: one record read.
 *	< 0	: EOF.
 *	< -1	: record rejected.
 * @desc	:
 *	read one row from file, using 'r' as record buffer, and 'md' as record
 *	meta data.
 */
int DSVReader::read(DSVRecord* r, List* list_md)
{
	int x		= 0;
	int n		= 0;
	size_t startp = _p;
	size_t len = 0;
	size_t chop_bgn = 0;
	size_t blob_size = 0;
	ssize_t s = 0;
	DSVRecordMD* rmd = NULL;

	if (_i == 0) {
		s = File::read();
		if (s == 0) {
			return 0;
		}
	} else if (startp >= _i) {
		startp	= startp - _p;
		s	= refill_buffer(0);
		if (s <= 0) {
			if (s == 0) {
				return 0;
			}
			goto reject;
		}
	}

	for (; x < list_md->size(); x++) {
		rmd = (DSVRecordMD*) list_md->at(x);

		if (rmd->_start_p) {
			len = _p + rmd->_start_p;
			if (len > _i) {
				s = refill_buffer(rmd->_start_p);
				if (s <= 0) {
					goto reject;
				}
			}
			startp = _p + rmd->_start_p;
		}
		if (rmd->_left_q) {
			if (_v[startp] != rmd->_left_q) {
				goto reject;
			}

			++startp;
			if (startp >= _i) {
				startp	= startp - _p;
				s	= refill_buffer(0);
				if (s <= 0) {
					goto reject;
				}
			}
		}
		if (rmd->_type == RMD_T_BLOB) {
			len = startp + DSVRecordMD::BLOB_SIZE;
			if (len >= _i) {
				startp	= startp - _p;
				s	= refill_buffer(DSVRecordMD::BLOB_SIZE);
				if (s <= 0) {
					goto reject;
				}
			}

			/* get record blob size */
			memcpy(&blob_size, &_v[startp], DSVRecordMD::BLOB_SIZE);

			r->set_len(blob_size);

			startp += DSVRecordMD::BLOB_SIZE;

			if (r->len() > _l) {
				startp	= startp - _p;
				s	= refill_buffer(r->len());
				if (s <= 0) {
					goto reject;
				}
			}

			len = startp + r->len();
			if (len >= _i) {
				startp	= startp - _p;
				s	= refill_buffer(r->len());
				if (s <= 0) {
					goto reject;
				}
			}

			/* copy blob from file buffer to record buffer */
			r->copy_raw(&_v[startp], r->len());

			startp += r->len();
			if (startp >= _i) {
				startp = startp - _p;
				s = refill_buffer(0);
				/* do not check for s == 0 */
				if (s < 0) {
					goto reject;
				}
			}
			if (rmd->_right_q) {
				if (_v[startp] != rmd->_right_q) {
					goto reject;
				}
				++startp;
				if (startp >= _i) {
					startp = startp - _p;
					s = refill_buffer(0);
					if (s <= 0) {
						goto reject;
					}
				}
			}
			if (rmd->_sep) {
				while (_v[startp] != rmd->_sep) {
					++startp;
					if (startp >= _i) {
						startp = startp - _p;
						s = refill_buffer(0);
						if (s <= 0) {
							goto reject;
						}
					}
				}

				++startp;
				if (startp >= _i) {
					startp = startp - _p;
					s = refill_buffer(0);
					/* do not check for s == 0 */
					if (s < 0) {
						goto reject;
					}
				}
			}
		} else { /* not BLOB */
			if (rmd->_end_p) {
				len = (startp - _p) + size_t(rmd->_end_p);

				if (len > r->size()) {
					r->resize(len);
				}

				if (startp + len >= _i) {
					len	= startp + len;
					startp	= startp - _p;

					s = refill_buffer(len);
					if (s <= 0) {
						goto reject;
					}
				}

				r->copy_raw(&_v[startp], len);
				startp += len;

			} else if (rmd->_right_q) {

				chop_bgn = startp;
				while (_v[startp] != rmd->_right_q) {
					++startp;
					if (startp >= _i) {
						startp		= startp - _p;
						chop_bgn	= chop_bgn - _p;

						s = refill_buffer(0);
						if (s <= 0) {
							goto reject;
						}
					}
				}

				len = startp - chop_bgn;
				if (len > 0) {
					s		= _v[startp];
					_v[startp]	= '\0';
					r->append_raw(&_v[chop_bgn], len);
					_v[startp]	= (char) s;
				}

				++startp;
				if (startp >= _i) {
					startp	= startp - _p;
					s	= refill_buffer(0);
					/* do not check for s == 0 */
					if (s < 0) {
						goto reject;
					}
				}

				if (rmd->_sep) {
					while (_v[startp] != rmd->_sep) {
						++startp;
						if (startp >= _i) {
							startp = startp - _p;
							s = refill_buffer(0);
							if (s <= 0) {
								goto reject;
							}
						}
					}

					++startp;
					if (startp >= _i) {
						startp = startp - _p;

						s = refill_buffer(0);
						/* do not check for s == 0 */
						if (s < 0) {
							goto reject;
						}
					}
				}
			} else if (rmd->_sep) {
				chop_bgn = startp;
				while (_v[startp] != rmd->_sep) {
					++startp;
					if (startp >= _i) {
						startp = startp - _p;
						chop_bgn = chop_bgn - _p;

						s = refill_buffer(0);
						if (s <= 0) {
							goto reject;
						}
					}
				}
				len = startp - chop_bgn;
				if (len > 0) {
					s		= _v[startp];
					_v[startp]	= '\0';
					r->append_raw(&_v[chop_bgn], len);
					_v[startp]	= (char) s;
				}

				++startp;
				if (startp >= _i) {
					startp = startp - _p;
					s = refill_buffer(0);
					/* do not check for s == 0 */
					if (s < 0) {
						goto reject;
					}
				}
			} else {
				chop_bgn = startp;
				while (_v[startp] != _eol) {
					++startp;
					if (startp >= _i) {
						startp = startp - _p;
						chop_bgn = chop_bgn - _p;

						s = refill_buffer(0);
						if (0 == s) {
							break;
						}
						if (s < 0) {
							goto reject;
						}
					}
				}

				len = startp - chop_bgn;
				if (len > 0) {
					s		= _v[startp];
					_v[startp]	= '\0';
					r->append_raw(&_v[chop_bgn], len);
					_v[startp]	= (char) s;
				}
			}
		}
		r	= r->_next_col;
		++n;
	}
	if (n == 0) {
		_p = startp;
		return 0;
	}
	if (x < list_md->size()) {
		goto reject;
	}
	while (_v[startp] != _eol) {
		++startp;
		if (startp >= _i) {
			startp	= startp - _p;
			s	= refill_buffer(0);
			if (s <= 0) {
				if (0 == s) {
					_p = startp;
					return 1;
				}
				goto reject;
			}
		}
	}
	_p = startp + 1;

	return 1;

reject:
	while (_v[startp] != _eol) {
		++startp;
		if (startp >= _i) {
			startp	= startp - _p;
			s	= refill_buffer(0);
			if (s <= 0) {
				if (0 == s) {
					_p = startp;
					return -1;
				}
				break;
			}
		}
	}
	_p = startp + 1;

	return -1;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
