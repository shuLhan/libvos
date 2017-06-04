//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "FmtParser.hh"

namespace vos {

const Error ErrInvalidConversion = Error("FmtParser: invalid conversion specifiers");
const Error ErrFlagDuplicate = Error("FmtParser: duplicate flag");
const Error ErrInvalidFormat = Error("FmtParser: invalid format string");

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
 * On success it will return NULL.
 *
 * On fail it will return,
 * - ErrFlagDuplicate if flag occured more than once.
 * - ErrInvalidFormat if formatted string is end without any conversion.
 * - ErrMemory if out of memory.
 */
Error FmtParser::parse_flag_chars()
{
	Error err;

	while (*_p) {
		switch (*_p) {
		case '-':
			if (_flag & FL_LEFT_ADJUST) {
				return ErrFlagDuplicate;
			}
			_flag |= FL_LEFT_ADJUST;
			// Disable 0 flag.
			_flag &= ~FL_ZERO_PAD;
			break;

		case '+':
			if (_flag & FL_SIGN) {
				return ErrFlagDuplicate;
			}
			_flag |= FL_SIGN;
			break;

		case '#':
			if (_flag & FL_ALT_OUT) {
				return ErrFlagDuplicate;
			}
			_flag |= FL_ALT_OUT;
			break;

		case '0':
			if (_flag & FL_ZERO_PAD) {
				return ErrFlagDuplicate;
			}
			if (! (_flag & FL_LEFT_ADJUST)) {
				_flag |= FL_ZERO_PAD;
			}
			break;

		default:
			return NULL;
		}

		err = _flags.appendc(*_p);
		if (err != NULL) {
			return err;
		}

		_p++;
	}

	if (! *_p) {
		return ErrInvalidFormat;
	}

	return NULL;
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
 * On fail it will return ErrRange and will NOT modified the pointer to
 * formatted string `_p`.
 */
Error FmtParser::parse_flag_width_prec()
{
	Error err;

	if (isdigit(*_p)) {
		err = Buffer::PARSE_INT(&_p, &_fwidth);
		if (err != NULL) {
			return err;
		}
	}
	if (*_p == '.') {
		_p++;
		_flag |= FL_PREC;

		if (isdigit(*_p)) {
			err = Buffer::PARSE_INT(&_p, &_fprec);
			if (err != NULL) {
				_fwidth = 0;
				_fprec = 0;
				return err;
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
 * On success it will return NULL, otherwise it will return
 * ErrInvalidConversion.
 */
Error FmtParser::check_flag_conversion(char c)
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

	return ErrInvalidConversion;
}

/**
 * Method `parse_flags(fmtp)` will parse '-+#0' characters, field width, and
 * precision from `fmt`; and save it to `flags` as bitmask of flag value
 * corresponding to the characters that found in `fmt`.
 *
 * '%' -> ['-'] ---> [digit] -> ['.'] -> [digit] -> ['h'] --> [conversion]
 *     \- ['+'] -/                               \- ['l'] -/
 *     \- ['#'] -/                               \- ['L'] -/
 *     \- ['0'] -/
 *
 * On success it will return NULL.
 *
 * On fail it will return ErrMemory.
 */
Error FmtParser::parse_flags()
{
	Error err;

	_flags.reset();
	_flag = 0;

	err = _flags.appendc('%');
	if (err != NULL) {
		return err;
	}

	err = parse_flag_chars();
	if (err != NULL) {
		return err;
	}

	err = parse_flag_width_prec();
	if (err != NULL) {
		return err;
	}

	parse_flag_length_mod();

	return check_flag_conversion(*_p);
}

Error FmtParser::parse_conversion()
{
	int i32;
	Error err;

	_conv.reset();

	switch (*_p) {
	case 'c':
		i32 = va_arg(_args, int);
		if (i32 > 0) {
			char c = char(i32);
			err = _conv.appendc(c);
			if (err != NULL) {
				return err;
			}
		}
		break;

	case 'd':
	case 'i':
		_flag |= FL_NUMBER;
		if (_flag & FL_LONG) {
			err =_conv.appendi(va_arg(_args, long int));
		} else {
			err =_conv.appendi(va_arg(_args, int));
		}
		if (err != NULL) {
			return err;
		}
		break;

	case 'u':
		_flag |= FL_NUMBER;
		if (_flag & FL_LONG) {
			err = _conv.appendui(va_arg(_args, long unsigned));
		} else {
			err = _conv.appendui(va_arg(_args, unsigned int));
		}
		if (err != NULL) {
			return err;
		}
		break;
	case 's':
		err = _conv.append_raw(va_arg(_args, const char*));
		if (err != NULL) {
			return err;
		}
		break;
	case 'f':
		_flag |= FL_NUMBER;
		err = _conv.appendd(va_arg(_args, double),
			_fprec ? _fprec : DEF_PREC);
		if (err != NULL) {
			return err;
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
		err = _conv.appendi(va_arg(_args, int), 8);
		if (err != NULL) {
			return err;
		}
		break;
	case 'p':
		_flag |= FL_ALT_OUT;
		__attribute__ ((fallthrough));
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
		err = _conv.appendi(va_arg(_args, int), 16);
		if (err != NULL) {
			return err;
		}
		break;
	}

	return 0;
}

void FmtParser::apply_flags_to_conversion()
{
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
			_fwidth = 0;
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

void FmtParser::on_invalid()
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
Error FmtParser::parse(const char *fmt, va_list args)
{
	Error err;

	reset();
	_p = (char *) fmt;
	va_copy(_args, args);

	while (*_p) {
		// (1.1)
		while (*_p && *_p != '%') {
			err = appendc(*_p);
			if (err != NULL) {
				return err;
			}

			_p++;
		}

		if (! *_p) {
			break;
		}

		_p++;

		if (! *_p) {
			err = appendc('%');
			if (err != NULL) {
				return err;
			}

			break;
		}

		// Escaped format '%%'
		if (*_p == '%') {
			err = appendc('%');
			if (err != NULL) {
				return err;
			}
			_p++;
			continue;
		}

		// (1.2)
		err = parse_flags();
		if (err != NULL) {
			on_invalid();
			continue;
		}

		err = parse_conversion();
		if (err != NULL) {
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

Error FmtParser::parse(const char *fmt, ...)
{
	if (!fmt) {
		return 0;
	}

	Error err;
	va_list args;

	va_start(args, fmt);
	err = parse(fmt, args);
	va_end(args);

	return err;
}

} // namespace::vos
