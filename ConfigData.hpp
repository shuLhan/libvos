/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#ifndef _LIBVOS_CONFIG_DATA_HPP
#define	_LIBVOS_CONFIG_DATA_HPP	1

#include "Buffer.hpp"

namespace vos {

#define	CFGDATA_DFLT_KEY_FMT	"KEY-"

enum _config_type {
	CONFIG_T_NONE	= 0,
	CONFIG_T_HEAD,
	CONFIG_T_KEY,
	CONFIG_T_VALUE,
	CONFIG_T_MISC
};

class ConfigData : public Buffer {
public:
	ConfigData();
	~ConfigData();

	int init(const int type, const char *data);

	void add_head(const ConfigData *head);
	int add_head_raw(const char *head);

	void add_key(const ConfigData *key);
	int add_key_raw(const char *key);

	int add_value(const ConfigData *value);
	int add_value_raw(const char *value);

	void add_misc(const ConfigData *misc);
	int add_misc_raw(const char *misc);

	void dump();

	static int INIT(ConfigData **o, const int type, const char *data);

	int		_t;
	ConfigData	*_value;
	ConfigData	*_next_head;
	ConfigData	*_last_head;
	ConfigData	*_next_key;
	ConfigData	*_last_key;
private:
	DISALLOW_COPY_AND_ASSIGN(ConfigData);
};

} /* namespace::vos */

#endif
