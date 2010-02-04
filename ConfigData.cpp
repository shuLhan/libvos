/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "ConfigData.hpp"

namespace vos {

const char * ConfigData::DFLT_KEY_FMT = "KEY-";

/**
 * @method	: ConfigData::ConfigData
 * @desc	: ConfigData constructor, initializing all attributes.
 */
ConfigData::ConfigData() :
	_t(0),
	_value(NULL),
	_next_head(NULL),
	_last_head(this),
	_next_key(NULL),
	_last_key(this)
{}

/**
 * @method	: ConfigData::~ConfigData
 * @desc	: release ConfigData object.
 */
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
 * @method	: ConfigData::init
 * @param	:
 *	> type	: type for a new ConfigData object.
 *	> data	: a contents for a ConfigData object.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: initialize ConfigData object.
 */
int ConfigData::init(const int type, const char *data)
{
	_t = type;
	return Buffer::init_raw(data, 0);
}

/**
 * @method	: ConfigData::add_head
 * @param	:
 *	> head	: pointer to new ConfigData header object.
 * @desc	: add new header to Config list.
 */
void ConfigData::add_head(const ConfigData *head)
{
	if (head) {
		_last_head->_next_head	= (ConfigData *) head;
		_last_head		= (ConfigData *) head;
	}
}

/**
 * @method	: ConfigData::add_head_raw
 * @param	:
 *	> head	: name of config header.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: add new header to Config list.
 */
int ConfigData::add_head_raw(const char *head)
{
	register int	s;
	ConfigData	*h = NULL;

	s = ConfigData::INIT(&h, CONFIG_T_HEAD, head);
	if (0 == s) {
		add_head(h);
	}
	return s;
}

/**
 * @method	: ConfigData::add_key
 * @param	:
 *	> key	: pointer to new ConfigData key object.
 * @desc	: add new key to the last header in Config list.
 */
void ConfigData::add_key(const ConfigData *key)
{
	if (key) {
		_last_head->_last_key->_next_key= (ConfigData *) key;
		_last_head->_last_key		= (ConfigData *) key;
	}
}

/**
 * @method	: ConfigData::add_key_raw
 * @param	:
 *	> key	: a name of the key.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: add new key to the last header in Config list.
 */
int ConfigData::add_key_raw(const char *key)
{
	register int	s;
	ConfigData	*k = NULL;

	s = ConfigData::INIT(&k, CONFIG_T_KEY, key);
	if (0 == s) {
		add_key(k);
	}
	return s;
}

/**
 * @method	: ConfigData::add_value
 * @param	:
 *	> value	: pointer to a new ConfigData value object.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: add value to the last key in the last header in Config list.
 */
int ConfigData::add_value(const ConfigData *value)
{
	if (!value)
		return 0;

	if ((CONFIG_T_KEY == _last_head->_last_key->_t)
	&&  NULL == _last_head->_last_key->_value) {
		_last_head->_last_key->_value = (ConfigData *) value;
	} else {
		int		s;
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
 * @method	: ConfigData::add_value_raw
 * @param	:
 *	> value : a string value for key.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: add value to the last key in the last header in Config list.
 */
int ConfigData::add_value_raw(const char *value)
{
	register int	s;
	ConfigData	*v = NULL;

	s = ConfigData::INIT(&v, CONFIG_T_VALUE, value);
	if (0 == s) {
		s = add_value(v);
	}
	return s;
}

/**
 * @method	: ConfigData::add_misc
 * @param	:
 *	> misc	: pointer to ConfigData object.
 * @desc	: add a non-key and non-header object, i.e: comment, to Config
 *                list.
 */
void ConfigData::add_misc(const ConfigData *misc)
{
	if (misc) {
		_last_head->_last_key->_next_key= (ConfigData *) (misc);
		_last_head->_last_key		= (ConfigData *) misc;
	}
}

/**
 * @method	: ConfigData::add_misc_raw
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: add a non-key and non-header object, i.e: comment, to Config
 *                list.
 */
int ConfigData::add_misc_raw(const char *misc)
{
	register int	s;
	ConfigData	*m = NULL;

	s = ConfigData::INIT(&m, CONFIG_T_MISC, misc);
	if (0 == s) {
		add_misc(m);
	}
	return s;
}

/**
 * @method	: ConfigData::dump
 * @desc	: dump content of ConfigData object to standard output,
 *                including the next header, key, and value.
 */
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
 * @method	: ConfigData::INIT
 * @param	:
 *	> o	: return value, pointer to unallocated ConfigData object.
 *	> type	: type for a new ConfigData object.
 *	> data	: content for a new ConfigData object.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: create and initialize a new ConfigData object.
 */
int ConfigData::INIT(ConfigData **o, const int type, const char *data)
{
	register int s = -1;

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
