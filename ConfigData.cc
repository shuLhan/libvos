//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "ConfigData.hh"

namespace vos {

const char* ConfigData::__cname = "ConfigData";

/**
 * @method	: ConfigData::ConfigData
 * @desc	: ConfigData constructor, initializing all attributes.
 */
ConfigData::ConfigData() : Buffer()
,	_t(0)
,	_value(NULL)
,	_next_head(NULL)
,	_last_head(this)
,	_next_key(NULL)
,	_last_key(this)
{}

/**
 * @method	: ConfigData::~ConfigData
 * @desc	: release ConfigData object.
 */
ConfigData::~ConfigData()
{
	if (CONFIG_T_KEY == _t) {
		if (_value) {
			delete _value;
			_value = NULL;
		}
	}
	if (_next_head) {
		delete _next_head;
		_next_head = NULL;
	}
	if (_next_key) {
		delete _next_key;
		_next_key = NULL;
	}
}

/**
 * @method	: ConfigData::init
 * @param	:
 *	> type	: type for a new ConfigData object.
 *	> data	: a contents for a ConfigData object.
 *	> len	: length of 'data', default to 0.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: initialize ConfigData object.
 */
int ConfigData::init(const int type, const char* data, size_t data_len)
{
	_t = type;
	return Buffer::copy_raw(data, data_len);
}

/**
 * @method	: ConfigData::add_head
 * @param	:
 *	> head	: pointer to new ConfigData header object.
 * @desc	: add new header to Config list.
 */
void ConfigData::add_head(const ConfigData* head)
{
	if (head) {
		_last_head->_next_head	= (ConfigData*) head;
		_last_head		= (ConfigData*) head;
	}
}

/**
 * @method	: ConfigData::add_head_raw
 * @param	:
 *	> head	: name of config header.
 *	> len	: length of string 'head', default to 0.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: add new header to Config list.
 */
int ConfigData::add_head_raw(const char* head, const size_t head_len)
{
	if (!head) {
		return -1;
	}

	ConfigData* h = NULL;

	int s = ConfigData::INIT(&h, CONFIG_T_HEAD, head, head_len);
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
void ConfigData::add_key(const ConfigData* key)
{
	if (key) {
		_last_head->_last_key->_next_key= (ConfigData*) key;
		_last_head->_last_key		= (ConfigData*) key;
	}
}

/**
 * @method	: ConfigData::add_key_raw
 * @param	:
 *	> key	: a name of the key.
 *	> len	: length of string 'key', default to 0.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: add new key to the last header in Config list.
 */
int ConfigData::add_key_raw(const char* key, const size_t key_len)
{
	if (!key) {
		return -1;
	}

	ConfigData* k = NULL;

	int s = ConfigData::INIT(&k, CONFIG_T_KEY, key, key_len);
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
 *	< -1	: fail.
 * @desc	: Add value to the last key in the last header in Config list.
 */
int ConfigData::add_value(const ConfigData* value)
{
	if (!value) {
		return -1;
	}
	if (CONFIG_T_KEY != _last_head->_last_key->_t) {
		return -1;
	}
	if (_last_head->_last_key->_value != NULL) {
		return -1;
	}
	_last_head->_last_key->_value = (ConfigData *) value;
	return 0;
}

/**
 * @method	: ConfigData::add_value_raw
 * @param	:
 *	> value : a string value for key.
 *	> len	: length of string 'value', default to 0.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: add value to the last key in the last header in Config list.
 */
int ConfigData::add_value_raw(const char* value, const size_t value_len)
{
	ConfigData* v = NULL;

	int s = ConfigData::INIT(&v, CONFIG_T_VALUE, value, value_len);
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
void ConfigData::add_misc(const ConfigData* misc)
{
	if (misc) {
		_last_head->_last_key->_next_key= (ConfigData *) (misc);
		_last_head->_last_key		= (ConfigData *) misc;
	}
}

/**
 * @method	: ConfigData::add_misc_raw
 * @param	:
 *	> misc	: string.
 *	> len	: length of string 'misc', default to 0.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: add a non-key and non-header object, i.e: comment, to Config
 *                list.
 */
int ConfigData::add_misc_raw(const char* misc, const size_t misc_len)
{
	if (!misc) {
		return -1;
	}

	ConfigData* m = NULL;

	int s = ConfigData::INIT(&m, CONFIG_T_MISC, misc, misc_len);
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
	Buffer		o;
	ConfigData*	h = this;
	ConfigData*	k = NULL;

	o.append_raw("[vos::CnfgData] dump: ----------------------------\n");
	while (h) {
		o.aprint("[%s]\n", h->_v);

		k = h->_next_key;
		while (k) {
			if (CONFIG_T_KEY == k->_t) {
				o.aprint("\t%s = %s\n", k->chars()
					, k->_value ? k->_value->chars()
						: "\0");
			} else {
				o.aprint("%s\n", k->chars());
			}
			k = k->_next_key;
		}
		h = h->_next_head;
	}
	o.append_raw("--------------------------------------------------\n");

	printf("%s", o.chars());
}

/**
 * @method	: ConfigData::INIT
 * @param	:
 *	> o	: return value, pointer to unallocated ConfigData object.
 *	> type	: type for a new ConfigData object.
 *	> data	: content for a new ConfigData object.
 *	> len	: length of string 'data'.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: create and initialize a new ConfigData object.
 */
int ConfigData::INIT(ConfigData** o, const int type, const char* data
			, const size_t data_len)
{
	int s = -1;

	(*o) = new ConfigData();
	if ((*o)) {
		s = (*o)->init(type, data, data_len);
		if (s < 0) {
			delete (*o);
			(*o) = NULL;
		}
	}
	return s;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
