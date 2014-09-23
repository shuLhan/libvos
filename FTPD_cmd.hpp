/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_FTP_COMMAND_HPP
#define	_LIBVOS_FTP_COMMAND_HPP	1

#include <ctype.h>
#include "FTP_cmd.hpp"

namespace vos {

/**
 * @class		: FTPD_cmd
 * @attr		:
 *	- _code		: command code.
 *	- _name		: command name.
 *	- _parm		: command parameter.
 *	- _callback	: pointer to function callback.
 *	- _next		: pointer to the next FTPD_cmd object.
 *	- _prev		: pointer to the previos FTPD_cmd object.
 * @desc		:
 * This class contain command code and parameter.
 */
class FTPD_cmd {
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
	FTPD_cmd*	_next;
	FTPD_cmd*	_last;

	static FTPD_cmd* INIT(const int code, const char* name
				, void (*callback)(const void*, const void*));
	static void ADD(FTPD_cmd** cmds, FTPD_cmd* cmd_new);
private:
	FTPD_cmd(const FTPD_cmd&);
	void operator=(const FTPD_cmd&);
};

} /* namespace::vos */

#endif
