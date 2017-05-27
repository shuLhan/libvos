//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBVOS_FMT_PARSER_HH
#define LIBVOS_FMT_PARSER_HH 1

#include "Buffer.hh"

namespace vos {

class FmtParser : public Buffer {
public:
	FmtParser();
	~FmtParser();

	int parse(const char *fmt, va_list args);
	int parse(const char *fmt, ...);

protected:
	Buffer  _flags;
	Buffer  _conv;
	int     _flag;
	int     _fwidth;
	int     _fprec;
	char    *_p;
	va_list _args;

private:
	FmtParser(const FmtParser&);
	void operator=(const FmtParser&);

	void reset();
	int parse_flag_chars();
	int parse_flag_width_prec();
	void parse_flag_length_mod();
	int check_flag_conversion(char c);
	int parse_flags();
	int parse_conversion();
	void apply_flags_to_conversion();
	void on_invalid();
};

} // namespace::vos

#endif
// vi: ts=8 sw=8 tw=78:
