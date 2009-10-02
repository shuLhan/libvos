/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "ConfigData.hpp"

namespace vos {

ConfigData::ConfigData(const int type) :
	_t(type),
	_value(NULL),
	_next_head(NULL),
	_last_head(this),
	_next_key(NULL),
	_last_key(this)
{}

ConfigData::ConfigData(const int type, const char *str) : Buffer(str),
	_t(type),
	_value(NULL),
	_next_head(NULL),
	_last_head(this),
	_next_key(NULL),
	_last_key(this)
{}

ConfigData::~ConfigData()
{
	if (CONFIG_T_KEY == _t) {
		if (_value)
			delete _value;
	}
	if (_next_head)
		delete _next_head;
	if (_next_key)
		delete _next_key;
}

void ConfigData::add_head(const ConfigData *head)
{
	_last_head->_next_head	= (ConfigData *) head;
	_last_head		= (ConfigData *) head;
}

void ConfigData::add_head(const char *head)
{
	ConfigData *h = new ConfigData(CONFIG_T_HEAD, head);
	add_head(h);
}

void ConfigData::add_key(const ConfigData *key)
{
	_last_head->_last_key->_next_key = (ConfigData *) key;
	_last_head->_last_key = (ConfigData *) key;
}

void ConfigData::add_key(const char *key)
{
	ConfigData *k = new ConfigData(CONFIG_T_KEY, key);
	add_key(k);
}

void ConfigData::add_value(const ConfigData *value)
{
	if ((CONFIG_T_KEY == _last_head->_last_key->_t)
	&&  ! _last_head->_last_key->_value) {
		_last_head->_last_key->_value = (ConfigData *) value;
	} else {
		int		i = 0;
		Buffer		key(CFGDATA_DFLT_KEY_FMT);
		ConfigData	*k = _last_head->_next_key;

		while (k) {
			++i;
			k = k->_next_key;
		}

		key.appendi(i, 10);
		add_key(key._v);
		add_value(value);
	}
}

void ConfigData::add_value(const char *value)
{
	ConfigData *v = new ConfigData(CONFIG_T_VALUE, value);
	add_value(v);
}

void ConfigData::add_misc(const ConfigData *misc)
{
	_last_head->_last_key->_next_key = (ConfigData *) (misc);
	_last_head->_last_key = (ConfigData *) misc;
}

void ConfigData::add_misc(const char *misc)
{
	ConfigData *m = new ConfigData(CONFIG_T_MISC, misc);
	add_misc(m);
}

void ConfigData::dump()
{
	ConfigData *h = this;
	ConfigData *k = NULL;

	while (h) {
		printf("[%s]\n", h->_v);

		k = h->_next_key;
		while (k) {
			if (CONFIG_T_KEY == k->_t) {
				printf("\t%s = %s\n", k->_v,
					k->_value ? k->_value->_v : "");
			} else {
				printf("%s\n", k->_v);
			}

			k = k->_next_key;
		}

		h = h->_next_head;
	}
}

} /* namespace::vos */
