//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_CONFIG_DATA_HH
#define _LIBVOS_CONFIG_DATA_HH 1

#include "Buffer.hh"

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

	Error init(const int type, const char* data, size_t data_len = 0);

	void add_head(const ConfigData* head);
	Error add_head_raw(const char* head, const size_t head_len = 0);

	void add_key(const ConfigData* key);
	Error add_key_raw(const char* key, const size_t key_len = 0);

	void add_value(const ConfigData* value);
	Error add_value_raw(const char* value, const size_t value_len = 0);

	void add_misc(const ConfigData* misc);
	Error add_misc_raw(const char* misc, const size_t misc_len = 0);

	void dump();

	int		_t;
	ConfigData*	_value;
	ConfigData*	_next_head;
	ConfigData*	_last_head;
	ConfigData*	_next_key;
	ConfigData*	_last_key;

	static Error INIT(ConfigData** o, const int type, const char* data
			, const size_t data_len = 0);

	static const char* __cname;
private:
	ConfigData(const ConfigData&);
	void operator=(const ConfigData&);
};

} /* namespace::vos */

#endif
// vi: ts=8 sw=8 tw=78:
