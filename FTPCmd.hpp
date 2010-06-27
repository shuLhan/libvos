/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_FTP_COMMAND_HPP
#define	_LIBVOS_FTP_COMMAND_HPP	1

#include <ctype.h>
#include "Buffer.hpp"

namespace vos {

enum _FTP_cmd_idx {
	FTP_ACCEPT	= 0,
	FTP_CMD_USER,
	FTP_CMD_PASS,
	FTP_CMD_SYST,
	FTP_CMD_TYPE,
	FTP_CMD_MODE,
	FTP_CMD_STRU,
	FTP_CMD_FEAT,
	FTP_CMD_SIZE,
	FTP_CMD_MDTM,
	FTP_CMD_CWD,
	FTP_CMD_CDUP,
	FTP_CMD_PASV,
	FTP_CMD_RETR,
	FTP_CMD_STOR,
	FTP_CMD_LIST,
	FTP_CMD_NLST,
	FTP_CMD_DELE,
	FTP_CMD_RNFR,
	FTP_CMD_RNTO,
	FTP_CMD_RMD,
	FTP_CMD_MKD,
	FTP_CMD_PWD,
	FTP_CMD_QUIT,
	N_FTP_CMD
};
extern const char* _FTP_cmd[N_FTP_CMD];

/**
 * @class		: FTPCmd
 * @attr		:
 *	- _code		: command code.
 *	- _name		: command name.
 *	- _parm		: command parameter.
 *	- _callback	: pointer to function callback.
 *	- _next		: pointer to the next FTPCmd object.
 *	- _prev		: pointer to the previos FTPCmd object.
 * @desc		:
 * This class contain command code and parameter.
 */
class FTPCmd {
public:
	FTPCmd();
	~FTPCmd();

	void reset();
	void set(FTPCmd *cmd);
	void dump();

	int	_code;
	Buffer	_name;
	Buffer	_parm;
	void	(*_callback)(const void* FTPD_p, const void* FTPClient_p);
	FTPCmd*	_next;
	FTPCmd*	_last;

	static FTPCmd* INIT(const int code, const char* name
				, void (*callback)(const void*, const void*));
	static void ADD(FTPCmd** cmds, FTPCmd* cmd_new);
private:
	FTPCmd(const FTPCmd&);
	void operator=(const FTPCmd&);
};

} /* namespace::vos */

#endif
