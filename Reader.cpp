/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Reader.hpp"

namespace vos {

/**
 * @method	: Reader::Reader
 * @desc	: Reader object constructor.
 */
Reader::Reader()
{}

/**
 * @method	: Reader::~Reader
 * @desc	: Reader object desctructor.
 */
Reader::~Reader()
{}

/**
 * @method		: Reader::refill_buffer
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
int Reader::refill_buffer(const int read_min)
{
	register int move_len	= 0;
	register int len	= 0;

	move_len = _i - _p;
	if (move_len > 0 && _p > ((_i / 2) + 1)) {
		if (LIBVOS_DEBUG) {
			printf("[libvos::Reader] refill_buffer: memmove: from %d of %d\n", _p, _i);
		}
		memmove(&_v[0], &_v[_p], move_len);
	}

	len = move_len + read_min;
	if (len > _l) {

		if (LIBVOS_DEBUG) {
			printf("\n reader resize: from %d to %d\n", _l, len);
		}

		resize(len);
		len -= move_len;
	} else {
		len = _l - move_len;
		if (len <= 0) {
			len = _l * 2;

			if (LIBVOS_DEBUG) {
				printf("\n reader resize: from %d to %d\n",
					_l, len);
			}

			resize(len);
			len -= move_len;
		}
	}

	_i = (int) ::read(_d, &_v[move_len], len);
	if (_i < 0) {
		return -1;
	}

	_i	+= move_len;
	_p	= 0;
	_v[_i]	= '\0';

	return _i;
}

/**
 * @method	: Reader::read
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
int Reader::read(Record* r, RecordMD* rmd)
{
	int n		= 0;
	int startp	= _p;
	int len		= 0;
	int s		= 0;
	int chop_bgn	= 0;

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

	while (rmd && r) {
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
			len = startp + RecordMD::BLOB_SIZE;
			if (len >= _i) {
				startp	= startp - _p;
				s	= refill_buffer(RecordMD::BLOB_SIZE);
				if (s <= 0) {
					goto reject;
				}
			}
			/* get record blob size */
			r->_i = 0;
			memcpy(&r->_i, &_v[startp], RecordMD::BLOB_SIZE);
			startp += RecordMD::BLOB_SIZE;

			if (r->_i > r->_l) {
				r->resize(r->_i + 1);
			}
			if (r->_i > _l) {
				startp	= startp - _p;
				s	= refill_buffer(r->_i);
				if (s <= 0) {
					goto reject;
				}
			}

			len = startp + r->_i;
			if (len >= _i) {
				startp	= startp - _p;
				s	= refill_buffer(r->_i);
				if (s <= 0) {
					goto reject;
				}
			}

			/* copy blob from file buffer to record buffer */
			memcpy(r->_v, &_v[startp], r->_i);
			r->_v[r->_i] = '\0';

			startp += r->_i;
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
				len = (startp - _p) + rmd->_end_p;

				if (len > r->_l) {
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

				memcpy(r->_v, &_v[startp], len);
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
		rmd	= rmd->_next;
		++n;
	}
	if (n == 0) {
		_p = startp;
		return 0;
	}
	if (rmd) {
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
