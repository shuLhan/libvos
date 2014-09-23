/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#ifndef _LIBVOS_CONFIG_HPP
#define	_LIBVOS_CONFIG_HPP	1

#include "File.hpp"
#include "ConfigData.hpp"

namespace vos {

enum _cfg_ch {
	CFG_CH_COMMENT		= '#'
,	CFG_CH_COMMENT2		= ';'
,	CFG_CH_CONT		= '\\'
,	CFG_CH_HEAD_OPEN	= '['
,	CFG_CH_HEAD_CLOSE	= ']'
,	CFG_CH_KEY_SEP		= '='
};

enum _cfg_save_mode {
	CONFIG_SAVE_WOUT_COMMENT	= 0
,	CONFIG_SAVE_WITH_COMMENT	= 1
};

#define	CONFIG_ROOT	"__CONFIG__"

/**
 * @class		: Config
 * @attr		:
 *	- _data		: list of config headers, keys, and values.
 * @desc		: module for reading config file in INI format.
 */
class Config : public File {
public:
	Config();
	~Config();

	int load(const char* ini);
	int save();
	int save_as(const char* ini, const int mode = CONFIG_SAVE_WOUT_COMMENT);
	void dump();
	void close();

	const char* get(const char* head, const char* key
				, const char* dflt = NULL);
	long int get_number(const char* head, const char* key
				, const int dflt = 0);

	int set(const char* head, const char* key, const char* value);

	int add(const char* head, const char* key, const char* value)
	{
		return set(head, key, value);
	}

	void add_comment(const char* comment);

	ConfigData _data;
private:
	Config(const Config&);
	void operator=(const Config&);

	int parsing();
};

} /* namespace::vos */

#endif
