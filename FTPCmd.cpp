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
	_code(0),
	_parm()
{}

FTPCmd::~FTPCmd()
{}

/**
 * @return	:
 *	< > 0	: success, reply code.
 *	< -1	: fail.
 */
int FTPCmd::get(Buffer* c)
{
	if (!c) {
		return -1;
	}

	int	s;
	Buffer	cmd;

	s = 0;
	while (s < c->_i && !isspace(c->_v[s])) {
		s++;
	}
	if (s == 0) {
		return -1;
	}

	cmd.copy_raw(&c->_v[0], s);

	s++;
	_parm.reset();
	if (s < c->_i) {
		_parm.copy_raw(&c->_v[s], c->_i - s);
		_parm.trim();
	}

	_code = 0;
	for (s = 0; s < N_FTP_CMD; s++) {
		if (cmd.like_raw(_FTP_cmd[s]) == 0) {
			_code = s;
			break;
		}
	}
	if (!_code) {
		fprintf(stderr, "[LIBVOS::FTPCmd__] Unknown command: %s\n", cmd._v);
		return -1;
	}

	return _code;
}

void FTPCmd::set(FTPCmd *cmd)
{
	_code = cmd->_code;
	_parm.copy(&cmd->_parm);
}

void FTPCmd::dump()
{
	printf("[LIBVOS::FTPCmd__] command   : %s\n", _FTP_cmd[_code]);
	printf("                   parameter : %s\n", _parm._v);
}

} /* namespace::vos */
