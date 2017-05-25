//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "FmtParser.hh"

namespace vos {

enum __print_flag {
	FL_LEFT_ADJUST	= (1 << 0)
,	FL_SIGN		= (1 << 1)
,	FL_ZERO_PAD	= (1 << 2)
,	FL_WIDTH	= (1 << 3)
,	FL_PREC		= (1 << 4)
,	FL_OCTAL	= (1 << 5)
,	FL_HEX		= (1 << 6)
,	FL_NUMBER	= (1 << 7)
,	FL_ALT_OUT	= (1 << 8)
,	FL_SHORT	= (1 << 9)
,	FL_LONG		= (1 << 10)
,	FL_LONG_DBL	= (1 << 11)
};

const int DEF_PREC = 6;

FmtParser::FmtParser()
: Buffer()
, _flags()
, _conv()
, _flag(0)
, _fwidth(0)
, _fprec(0)
, _p(0)
, _args()
{}

FmtParser::~FmtParser()
{
	va_end(_args);
	_p = NULL;
}

void FmtParser::reset()
{
	Buffer::reset();
	_flags.reset();
	_conv.reset();
	_flag = 0;
	_fwidth = 0;
	_fprec = 0;
	_p = 0;
	va_end(_args);
}

/**
 * Method `parse_flag_chars(p, flags)` will parse `-+#0` characters from
 * formatted string `fmt`. Those characters are optional.
 *
 * '%' -> ['-'] ---> ...
 *     \- ['+'] -/
 *     \- ['#'] -/
 *     \- ['0'] -/
 *
 * The flag character `-`, `+`, `#`, and `0` only allowed once.
 *
 * On success it will return 0, otherwise it will return `-1`.
 */
int FmtParser::parse_flag_chars()
{
	while (*_p) {
		switch (*_p) {
		case '-':
			if (_flag & FL_LEFT_ADJUST) {
				return -1;
			}
			_flag |= FL_LEFT_ADJUST;
			// Disable 0 flag.
			_flag &= ~FL_ZERO_PAD;
			break;

		case '+':
			if (_flag & FL_SIGN) {
				return -1;
			}
			_flag |= FL_SIGN;
			break;

		case '#':
			if (_flag & FL_ALT_OUT) {
				return -1;
			}
			_flag |= FL_ALT_OUT;
			break;

		case '0':
			if (_flag & FL_ZERO_PAD) {
				return -1;
			}
			if (! (_flag & FL_LEFT_ADJUST)) {
				_flag |= FL_ZERO_PAD;
			}
			break;

		default:
			return 0;
		}

		_flags.appendc(*_p);
		_p++;
	}

	if (! *_p) {
		return -1;
	}

	return 0;
}

/**
 * Method `parse_flag_width_prec(fmtp)` will parse width
 * and precision value from formatted string `fmt` into `fwlen` and `fprec`.
 *
 * Width and precision is optional flags.
 *
 * ```
 * '%' -> ... ---> [width] -> ['.'] -> [precision] -> ...
 * ```
 *
 * On success it will return 0 and modified the pointer of formatted string
 * `_p`.
 *
 * On fail it will return `-1` and will NOT modified the pointer to formatted
 * string `_p`.
 */
int FmtParser::parse_flag_width_prec()
{
	if (isdigit(*_p)) {
		int s = Buffer::PARSE_INT(&_p, &_fwidth);
		if (s) {
			return -1;
		}
	}
	if (*_p == '.') {
		_p++;
		_flag |= FL_PREC;

		if (isdigit(*_p)) {
			int s = Buffer::PARSE_INT(&_p, &_fprec);
			if (s) {
				_fwidth = 0;
				_fprec = 0;
				return -1;
			}
		}
	}

	return 0;
}

/**
 * Method `parse_flag_length_mod(fmt, flags)` will parse length modifier for
 * integer conversion.
 *
 * '%'-> ...--> ['h'] --> [conversion]
 *           \- ['l'] -/
 *           \- ['L'] -/
 *
 * On success it will non-zero flag value and modified the pointer to
 * formatted string.
 * When no length modifier appeared on `fmt`, then it will return 0.
 */
void FmtParser::parse_flag_length_mod()
{
	switch (*_p) {
	case 'h':
		_flag |= FL_SHORT;
		_flags.appendc(*_p);
		_p++;
		break;
	case 'l':
		_flag |= FL_LONG;
		_flags.appendc(*_p);
		_p++;
		break;
	case 'L':
		_flag |= FL_LONG_DBL;
		_flags.appendc(*_p);
		_p++;
		break;
	}
}

/**
 * Method `check_flag_conversion(c)` will check if `c` is conformed to one of
 * the format conversion.
 *
 * On success it will return 0, otherwise it will return -1.
 */
int FmtParser::check_flag_conversion(char c)
{
	switch (c) {
	case 'c':
	case 'd': case 'i':
	case 'f':
	case 'o':
	case 'p':
	case 's':
	case 'u':
	case 'x': case 'X':
		return 0;
	}

	return -1;
}

/**
 * Method `parse_flags(fmtp)` will parse '-+#0' characters, field width, and
 * precision from `fmt`; and save it to `flags` as bitmask of flag value
 * corresponding to the characters that found in `fmt`.
 *
 * It will return `0` on success, `1` if flags is `%%`, `-1` if error occured.
 *
 * '%' -> ['-'] ---> [digit] -> ['.'] -> [digit] -> ['h'] --> [conversion]
 *     \- ['+'] -/                               \- ['l'] -/
 *     \- ['#'] -/                               \- ['L'] -/
 *     \- ['0'] -/
 */
int FmtParser::parse_flags()
{
	int s;

	_flags.reset();
	_flag = 0;

	// '%'
	_flags.appendc(*_p);
	_p++;

	// Escaped format '%%'
	if (*_p == '%') {
		_p++;
		return 1;
	}

	s = parse_flag_chars();
	if (s < 0) {
		return s;
	}

	s = parse_flag_width_prec();
	if (s < 0) {
		return s;
	}

	parse_flag_length_mod();

	return check_flag_conversion(*_p);
}

int FmtParser::parse_conversion()
{
	char c;
	int s, i32;

	_conv.reset();

	switch (*_p) {
	case 'c':
		i32 = va_arg(_args, int);
		if (i32 > 0) {
			c = char(i32);
			s = _conv.appendc(c);
			if (s < 0) {
				return -1;
			}
		}
		break;

	case 'd':
	case 'i':
		_flag |= FL_NUMBER;
		if (_flag & FL_LONG) {
			s =_conv.appendi(va_arg(_args, long int));
		} else {
			s =_conv.appendi(va_arg(_args, int));
		}
		if (s < 0) {
			return -1;
		}
		break;

	case 'u':
		_flag |= FL_NUMBER;
		if (_flag & FL_LONG) {
			s = _conv.appendui(va_arg(_args, long unsigned));
		} else {
			s = _conv.appendui(va_arg(_args, unsigned int));
		}
		if (s < 0) {
			return -1;
		}
		break;
	case 's':
		s = _conv.append_raw(va_arg(_args, const char*));
		if (s < 0) {
			return -1;
		}
		break;
	case 'f':
		_flag |= FL_NUMBER;
		s = _conv.appendd(va_arg(_args, double),
			_fprec ? _fprec : DEF_PREC);
		if (s < 0) {
			return -1;
		}
		break;
	case 'o':
		_flag |= FL_OCTAL | FL_NUMBER;
		_flag &= ~FL_SIGN;
		if (_fwidth) {
			if (_flag & FL_ALT_OUT) {
				--_fwidth;
			}
		}
		s = _conv.appendi(va_arg(_args, int), 8);
		if (s < 0) {
			return -1;
		}
		break;
	case 'p':
		_flag |= FL_ALT_OUT;
	case 'x':
	case 'X':
		_flag |= FL_HEX | FL_NUMBER;
		_flag &= ~FL_SIGN;
		if (_fwidth >= 2) {
			if (_flag & FL_ALT_OUT) {
				_fwidth -= 2;
			}
		} else {
			_fwidth = 0;
		}
		s = _conv.appendi(va_arg(_args, int), 16);
		if (s < 0) {
			return -1;
		}
		break;
	}

	return 0;
}

void FmtParser::apply_flags_to_conversion()
{
	if (! _flag) {
		return;
	}

	int convlen = int(_conv.len());

	if (_fwidth > convlen) {
		_fwidth = _fwidth - convlen;
	} else {
		_fwidth = 0;
	}

	if (_flag & FL_NUMBER) {
		if (_flag & FL_ZERO_PAD) {
			if (_fwidth > 0) {
				_conv.shiftr(size_t(_fwidth), '0');
				_fwidth = 0;
			}
		}
		if (_flag & FL_SIGN) {
			if (_conv.char_at(0) != '-') {
				_conv.shiftr(1);
				_conv.set_char_at(0, '+');
				_fwidth--;
			}
		}
	}

	if (_fwidth > 0) {
		if (_flag & FL_LEFT_ADJUST) {
			while (_fwidth) {
				_conv.appendc(' ');
				_fwidth--;
			}
		} else {
			_conv.shiftr(size_t(_fwidth), ' ');
		}
	}

	if (_flag & FL_ALT_OUT) {
		if (_flag & FL_OCTAL) {
			_conv.shiftr(1, '0');
		} else if (_flag & FL_HEX) {
			_conv.shiftr(2, 'x');
			_conv.set_char_at(0, '0');
		}
	}

	_fwidth = 0;
}

int FmtParser::on_invalid()
{
	append(&_flags);

	if (_fwidth > 0) {
		appendi(_fwidth);
	}
	if (_flag & FL_PREC) {
		appendc('.');
	}
	if (_fprec > 0) {
		appendi(_fprec);
	}
}

//
// (1) For each character in `fmt`
// (1.1) If its not `%` append it to temporary buffer `b`.
// (1.2) If its `%` start parsing for flags
//
int FmtParser::parse(const char *fmt, va_list args)
{
	int s;

	reset();
	_p = (char *) fmt;
	va_copy(_args, args);

	while (*_p) {
		// (1.1)
		while (*_p && *_p != '%') {
			s = appendc(*_p);
			if (s < 0) {
				return -1;
			}

			_p++;
		}

		if (! *_p) {
			break;
		}

		// (1.2)
		s = parse_flags();
		if (s) {
			on_invalid();
			continue;
		}

		s = parse_conversion();
		if (s) {
			on_invalid();
			continue;
		}

		apply_flags_to_conversion();

		append(&_conv);

		_p++;
	}

	va_end(_args);

	return 0;
}

int FmtParser::parse(const char *fmt, ...)
{
	if (!fmt) {
		return 0;
	}

	int s;
	va_list args;

	va_start(args, fmt);
	s = parse(fmt, args);
	va_end(args);

	return s;
}

} // namespace::vos
