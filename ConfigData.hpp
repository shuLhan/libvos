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
	explicit ConfigData(const int type);
	explicit ConfigData(const int type, const char *str);
	~ConfigData();

	void add_head(const ConfigData *head);
	void add_head(const char *head);

	void add_key(const ConfigData *key);
	void add_key(const char *key);

	void add_value(const ConfigData *value);
	void add_value(const char *value);

	void add_misc(const ConfigData *misc);
	void add_misc(const char *misc);

	void dump();

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
