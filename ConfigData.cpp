/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "ConfigData.hpp"

namespace vos {

const char * ConfigData::DFLT_KEY_FMT = "KEY-";

ConfigData::ConfigData() :
	_t(0),
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

/**
 * @desc	: initialize ConfigData object.
 *
 * @param	:
 *	> type	: type for a new ConfigData object.
 *	> data	: a contents for a ConfigData object.
 *
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int ConfigData::init(const int type, const char *data)
{
	_t = type;
	return Buffer::init_raw(data, 0);
}

/**
 * @desc	: add new head to Config list.
 */
void ConfigData::add_head(const ConfigData *head)
{
	if (head) {
		_last_head->_next_head	= (ConfigData *) head;
		_last_head		= (ConfigData *) head;
	}
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int ConfigData::add_head_raw(const char *head)
{
	int		s;
	ConfigData	*h = NULL;

	s = ConfigData::INIT(&h, CONFIG_T_HEAD, head);
	if (0 == s) {
		add_head(h);
	}
	return s;
}

void ConfigData::add_key(const ConfigData *key)
{
	if (key) {
		_last_head->_last_key->_next_key= (ConfigData *) key;
		_last_head->_last_key		= (ConfigData *) key;
	}
}

int ConfigData::add_key_raw(const char *key)
{
	int		s;
	ConfigData	*k = NULL;

	s = ConfigData::INIT(&k, CONFIG_T_KEY, key);
	if (0 == s) {
		add_key(k);
	}
	return s;
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int ConfigData::add_value(const ConfigData *value)
{
	int s;

	if (!value)
		return 0;

	if ((CONFIG_T_KEY == _last_head->_last_key->_t)
	&&  ! _last_head->_last_key->_value) {
		_last_head->_last_key->_value = (ConfigData *) value;
	} else {
		int		i = 0;
		Buffer		key;
		ConfigData	*k = _last_head->_next_key;

		s = key.init_raw(ConfigData::DFLT_KEY_FMT, 0);
		if (s < 0)
			return s;

		while (k) {
			++i;
			k = k->_next_key;
		}

		s = key.appendi(i);
		if (s < 0)
			return s;

		add_key_raw(key._v);
		add_value(value);
	}
	return 0;
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int ConfigData::add_value_raw(const char *value)
{
	int		s;
	ConfigData	*v = NULL;

	s = ConfigData::INIT(&v, CONFIG_T_VALUE, value);
	if (0 == s) {
		s = add_value(v);
	}
	return s;
}

void ConfigData::add_misc(const ConfigData *misc)
{
	if (misc) {
		_last_head->_last_key->_next_key= (ConfigData *) (misc);
		_last_head->_last_key		= (ConfigData *) misc;
	}
}

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int ConfigData::add_misc_raw(const char *misc)
{
	int		s;
	ConfigData	*m = NULL;

	s = ConfigData::INIT(&m, CONFIG_T_MISC, misc);
	if (0 == s) {
		add_misc(m);
	}
	return s;
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

/**
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 */
int ConfigData::INIT(ConfigData **o, const int type, const char *data)
{
	int s = -E_MEM;

	(*o) = new ConfigData();
	if ((*o)) {
		s = (*o)->init(type, data);
		if (s < 0) {
			delete (*o);
			(*o) = NULL;
		}
	}
	return s;
}

} /* namespace::vos */
