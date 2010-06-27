/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "FTPCmd.hpp"

namespace vos {

const char *_FTP_cmd[N_FTP_CMD] = {
	"\0",
	"USER",	/* USER	username		*/
	"PASS",	/* PASS	password		*/
	"SYST", /* SYST				*/
	"TYPE",	/* TYPE	[A|I]			*/
	"MODE",	/* MODE [S]			*/
	"STRU", /* STRU [F]			*/
	"FEAT", /* FEAT				*/
	"SIZE", /* SIZE /path/			*/
	"MDTM",	/* MDTM /path/			*/
	"CWD" ,	/* CWD	directory		*/
	"CDUP",	/* CDUP				*/
	"PASV",	/* PASV				*/
	"RETR",	/* RETR	/path/to/filename	*/
	"STOR",	/* STOR	/path/to/filename	*/
	"LIST",	/* LIST	[directory]		*/
	"NLST",	/* NLST	[directory]		*/
	"DELE",	/* DELE	/path/to/filename	*/
	"RNFR",	/* RNFR	/path/to/filename.old	*/
	"RNTO",	/* RNTO	/path/to/filename.new	*/
	"RMD" ,	/* RMD	directory		*/
	"MKD" ,	/* MKD	directory		*/
	"PWD" ,	/* PWD				*/
	"QUIT"	/* QUIT				*/
};

FTPCmd::FTPCmd() :
	_code(0)
,	_name()
,	_parm()
,	_callback(NULL)
,	_next(NULL)
,	_last(this)

{}

FTPCmd::~FTPCmd()
{
	if (_next) {
		delete _next;
	}
	reset();
}

/**
 * @method	: FTPCmd::reset
 * @desc	: reset all attribute.
 */
void FTPCmd::reset()
{
	_code = 0;
	_name.reset();
	_parm.reset();
	_callback = 0;
}

/**
 * @method	: FTPCmd::set
 * @param	:
 *	> cmd	: pointer to FTPCmd object.
 * @desc	: set content of this object using data from 'cmd' object.
 */
void FTPCmd::set(FTPCmd *cmd)
{
	_code = cmd->_code;
	_name.copy(&cmd->_name);
	_parm.copy(&cmd->_parm);
	_callback = cmd->_callback;
}

/**
 * @method	: FTPCmd::dump
 * @desc	: Dump content of FTPCmd object.
 */
void FTPCmd::dump()
{
	printf("[LIBVOS::FTPCmd__] command   : %s\n", _name._v);
	printf("                   parameter : %s\n", _parm._v);
}

/**
 * @method		: FTPCmd::INIT
 * @param		:
 *	> name		: name of new command.
 * @return		: pointer to function.
 *	< !NULL		: success, pointer to new FTPCmd object.
 *	< NULL		: fail.
 * @desc		: Create and initialize a new FTPCmd object.
 */
FTPCmd* FTPCmd::INIT(const int code, const char* name
			, void (*callback)(const void*, const void*))
{
	FTPCmd* cmd = new FTPCmd();
	if (cmd) {
		cmd->_code	= code;
		cmd->_callback	= callback;
		cmd->_name.copy_raw(name);
	}
	return cmd;
}

/**
 * @method		: FTPCmd::ADD
 * @param		:
 *	> cmds		: pointer to list of FTPCmd object.
 *	> cmd_new	: pointer to FTPCmd object.
 * @desc		: Add new command 'cmd_new' to list of command 'cmds'.
 */
void FTPCmd::ADD(FTPCmd** cmds, FTPCmd* cmd_new)
{
	if (!cmd_new) {
		return;
	}
	if (!(*cmds)) {
		(*cmds) = cmd_new;
	} else {
		(*cmds)->_last->_next = cmd_new;
	}
	(*cmds)->_last = cmd_new;
}

} /* namespace::vos */
