//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_FTP_COMMAND_LIST_HH
#define _LIBVOS_FTP_COMMAND_LIST_HH 1

#include "Socket.hh"

namespace vos {

enum _FTP_cmd_idx {
	FTP_ACCEPT	= 0
,	FTP_CMD_USER
,	FTP_CMD_PASS
,	FTP_CMD_ACCT
,	FTP_CMD_SYST
,	FTP_CMD_TYPE
,	FTP_CMD_MODE
,	FTP_CMD_STRU
,	FTP_CMD_FEAT
,	FTP_CMD_SIZE
,	FTP_CMD_MDTM
,	FTP_CMD_CWD
,	FTP_CMD_CDUP
,	FTP_CMD_PWD
,	FTP_CMD_PASV
,	FTP_CMD_LIST
,	FTP_CMD_NLST
,	FTP_CMD_RETR
,	FTP_CMD_STOR
,	FTP_CMD_DELE
,	FTP_CMD_RNFR
,	FTP_CMD_RNTO
,	FTP_CMD_RMD
,	FTP_CMD_MKD
,	FTP_CMD_QUIT
,	N_FTP_CMD
};
extern const char* _FTP_cmd[N_FTP_CMD];

enum _ftp_stat {
	FTP_STT_DISCONNECT	= vos::FILE_OPEN_NO
,	FTP_STT_CONNECTED	= O_RDWR
,	FTP_STT_LOGGED_IN	= O_RDWR << 1
,	FTP_STT_LOGGED_OUT	= O_RDWR << 2
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
