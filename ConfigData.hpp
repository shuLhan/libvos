/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_CONFIG_DATA_HPP
#define	_LIBVOS_CONFIG_DATA_HPP	1

#include "Buffer.hpp"

namespace vos {

enum _config_type {
	CONFIG_T_NONE	= 0
,	CONFIG_T_HEAD
,	CONFIG_T_KEY
,	CONFIG_T_VALUE
,	CONFIG_T_MISC
};

/**
 * @class		: ConfigData
 * @attr		:
 *	- _t		: type of config data.
 *	- _value	: value of config data object.
 *	- _next_head	: pointer to the next header.
 *	- _last_head	: pointer to the last header.
 *	- _next_key	: pointer to the next key.
 *	- _last_key	: pointer to the last key.
 * @desc		:
 *	ConfigData is a class that mapping all header, key, value, and
 *	comments from ini config file to list of object in memory.
 */
class ConfigData : public Buffer {
public:
	ConfigData();
	~ConfigData();

	int init(const int type, const char* data, int len = 0);

	void add_head(const ConfigData* head);
	int add_head_raw(const char* head, int len = 0);

	void add_key(const ConfigData* key);
	int add_key_raw(const char* key, int len = 0);

	int add_value(const ConfigData* value);
	int add_value_raw(const char* value, int len = 0);

	void add_misc(const ConfigData* misc);
	int add_misc_raw(const char* misc, int len = 0);

	void dump();

	static int INIT(ConfigData** o, const int type, const char* data
			, int len = 0);

	int		_t;
	ConfigData*	_value;
	ConfigData*	_next_head;
	ConfigData*	_last_head;
	ConfigData*	_next_key;
	ConfigData*	_last_key;
private:
	ConfigData(const ConfigData&);
	void operator=(const ConfigData&);
};

} /* namespace::vos */

#endif
