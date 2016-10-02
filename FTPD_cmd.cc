//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "FTPD_cmd.hh"

namespace vos {

const char* FTPD_cmd::__cname = "FTPD_cmd";

FTPD_cmd::FTPD_cmd() :
	_code(0)
,	_name()
,	_parm()
,	_callback(NULL)

{}

FTPD_cmd::~FTPD_cmd()
{
	reset();
}

/**
 * @method	: FTPD_cmd::reset
 * @desc	: reset all attribute.
 */
void FTPD_cmd::reset()
{
	_code = 0;
	_name.reset();
	_parm.reset();
	_callback = 0;
}

/**
 * @method	: FTPD_cmd::set
 * @param	:
 *	> cmd	: pointer to FTPD_cmd object.
 * @desc	: set content of this object using data from 'cmd' object.
 */
void FTPD_cmd::set(FTPD_cmd *cmd)
{
	_code = cmd->_code;
	_name.copy(&cmd->_name);
	_parm.copy(&cmd->_parm);
	_callback = cmd->_callback;
}

/**
 * @method	: FTPD_cmd::dump
 * @desc	: Dump content of FTPD_cmd object.
 */
void FTPD_cmd::dump()
{
	printf(	"[vos::FTPD_cmd__] dump:\n"
		"  command   : %s\n"
		"  parameter : %s\n", _name.chars(), _parm.chars());
}

/**
 * @method		: FTPD_cmd::INIT
 * @param		:
 *	> name		: name of new command.
 * @return		: pointer to function.
 *	< !NULL		: success, pointer to new FTPD_cmd object.
 *	< NULL		: fail.
 * @desc		: Create and initialize a new FTPD_cmd object.
 */
FTPD_cmd* FTPD_cmd::INIT(const int code, const char* name
			, void (*callback)(const void*, const void*))
{
	FTPD_cmd* cmd = new FTPD_cmd();
	if (cmd) {
		cmd->_code	= code;
		cmd->_callback	= callback;
		cmd->_name.copy_raw(name);
	}
	return cmd;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
