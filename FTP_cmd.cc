//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "FTP_cmd.hh"

namespace vos {

const char *_FTP_cmd[N_FTP_CMD] = {
	"\0"
,	"USER"	/* USER	username		*/
,	"PASS"	/* PASS	password		*/
,	"ACCT"	/* ACCT accountname		*/
,	"SYST"	/* SYST				*/
,	"TYPE"	/* TYPE	[A|I]			*/
,	"MODE"	/* MODE [S]			*/
,	"STRU"	/* STRU [F]			*/
,	"FEAT"	/* FEAT				*/
,	"SIZE"	/* SIZE /path/			*/
,	"MDTM"	/* MDTM /path/			*/
,	"CWD"	/* CWD	directory		*/
,	"CDUP"	/* CDUP				*/
,	"PWD"	/* PWD				*/
,	"PASV"	/* PASV				*/
,	"LIST"	/* LIST	[directory]		*/
,	"NLST"	/* NLST	[directory]		*/
,	"RETR"	/* RETR	/path/to/filename	*/
,	"STOR"	/* STOR	/path/to/filename	*/
,	"DELE"	/* DELE	/path/to/filename	*/
,	"RNFR"	/* RNFR	/path/to/filename.old	*/
,	"RNTO"	/* RNTO	/path/to/filename.new	*/
,	"RMD"	/* RMD	directory		*/
,	"MKD"	/* MKD	directory		*/
,	"QUIT"	/* QUIT				*/
};



} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
