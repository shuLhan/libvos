//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_FTP_COMMAND_HH
#define _LIBVOS_FTP_COMMAND_HH 1

#include <ctype.h>
#include "FTP_cmd.hh"

namespace vos {

/**
 * @class		: FTPD_cmd
 * @attr		:
 *	- _code		: command code.
 *	- _name		: command name.
 *	- _parm		: command parameter.
 *	- _callback	: pointer to function callback.
 * @desc		:
 * This class contain command code and parameter.
 */
class FTPD_cmd : public Object {
public:
	FTPD_cmd();
	~FTPD_cmd();

	void reset();
	void set(FTPD_cmd *cmd);
	void dump();

	int	_code;
	Buffer	_name;
	Buffer	_parm;
	void	(*_callback)(const void* FTPD_p, const void* FTPD_client_p);

	static FTPD_cmd* INIT(const int code, const char* name
				, void (*callback)(const void*, const void*));

	static const char* __cname;
private:
	FTPD_cmd(const FTPD_cmd&);
	void operator=(const FTPD_cmd&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
