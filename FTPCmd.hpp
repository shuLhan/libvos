/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_FTP_COMMAND_HPP
#define	_LIBVOS_FTP_COMMAND_HPP	1

#include <ctype.h>
#include "Socket.hpp"

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

class FTPCmd {
public:
	FTPCmd();
	~FTPCmd();

	int get(Buffer* c);
	void set(FTPCmd *cmd);
	void dump();

	int	_code;
	Buffer	_parm;
private:
	FTPCmd(const FTPCmd&);
	void operator=(const FTPCmd&);
};

} /* namespace::vos */

#endif
