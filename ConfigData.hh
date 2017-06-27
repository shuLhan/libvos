//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_CONFIG_DATA_HH
#define _LIBVOS_CONFIG_DATA_HH 1

#include "Buffer.hh"

namespace vos {

enum CONFIG_TYPE {
	CONFIG_T_NONE	= 0
,	CONFIG_T_HEAD
,	CONFIG_T_KEY
,	CONFIG_T_VALUE
,	CONFIG_T_MISC
};

/**
 * Class ConfigData represent a mapping of header, key, value, and
 * comments from ini config file to list of object.
 *
 * Field type contains type of config data.
 * Field value	contains value of config data object.
 * Field next_head contains pointer to the next header.
 * Field last_head contains pointer to the last header.
 * Field next_key contains pointer to the next key.
 * Field last_key contains pointer to the last key.
 */
class ConfigData : public Buffer {
public:
	static const char* __CNAME;

	size_t type;
	ConfigData* value;
	ConfigData* next_head;
	ConfigData* last_head;
	ConfigData* next_key;
	ConfigData* last_key;

	ConfigData(enum CONFIG_TYPE type, const char* data
		, const size_t data_len = 0);
	~ConfigData();

	void add_head(const ConfigData* head);
	void add_head_raw(const char* head, const size_t head_len = 0);

	void add_key(const ConfigData* key);
	void add_key_raw(const char* key, const size_t key_len = 0);

	void add_value(const ConfigData* value);
	void add_value_raw(const char* value, const size_t value_len = 0);

	void add_misc(const ConfigData* misc);
	void add_misc_raw(const char* misc, const size_t misc_len = 0);

	const char* chars();

private:
	ConfigData(const ConfigData&);
	void operator=(const ConfigData&);
};

} // namespace::vos

#endif

// vi: ts=8 sw=8 tw=80:
