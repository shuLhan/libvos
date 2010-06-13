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

void FTPCmd::reset()
{
	_code = 0;
	_name.reset();
	_parm.reset();
	_callback = 0;
}

void FTPCmd::set(FTPCmd *cmd)
{
	_code = cmd->_code;
	_name.copy(&cmd->_name);
	_parm.copy(&cmd->_parm);
	_callback = cmd->_callback;
}

void FTPCmd::dump()
{
	printf("[LIBVOS::FTPCmd__] command   : %s\n", _name._v);
	printf("                   parameter : %s\n", _parm._v);
}

/**
 * @param		:
 *	> name		: name of new command.
 * @return		: pointer to function.
 *	< !NULL		: success, pointer to new FTPCmd.
 *	< NULL		: fail.
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
