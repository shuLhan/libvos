//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "ConfigData.hh"

namespace vos {

const char* ConfigData::__CNAME = "ConfigData";

/**
 * Method ConfigData(type,data,data_len) will create and initialize ConfigData
 * object with `type` and `data`.
 */
ConfigData::ConfigData(enum CONFIG_TYPE type, const char* data
	, const size_t data_len)
: Buffer(data, data_len)
, type(type)
, value(NULL)
, next_head(NULL)
, last_head(this)
, next_key(NULL)
, last_key(this)
{}

/**
 * Method ~ConfigData() will release ConfigData object from memory.
 */
ConfigData::~ConfigData()
{
	if (CONFIG_T_KEY == type) {
		if (value) {
			delete value;
			value = NULL;
		}
	}
	if (next_head) {
		delete next_head;
		next_head = NULL;
	}
	if (next_key) {
		delete next_key;
		next_key = NULL;
	}
}

/**
 * Method add_head(head) will add new header `head` to Config list.
 */
void ConfigData::add_head(const ConfigData* head)
{
	if (head) {
		last_head->next_head	= (ConfigData*) head;
		last_head		= (ConfigData*) head;
	}
}

/**
 * Method `add_head_raw(head, head_len)` will add new header named `head` with
 * length is `head_len` to Config list.
 */
void ConfigData::add_head_raw(const char* head, const size_t head_len)
{
	if (head) {
		ConfigData* h = new ConfigData(CONFIG_T_HEAD, head, head_len);
		add_head(h);
	}
}

/**
 * Method add_key(key) will add new key to the last header in Config list.
 */
void ConfigData::add_key(const ConfigData* key)
{
	if (key) {
		last_head->last_key->next_key= (ConfigData*) key;
		last_head->last_key		= (ConfigData*) key;
	}
}

/**
 * Method add_key_raw(key,key_len) will add new key to the last header in
 * Config list.
 */
void ConfigData::add_key_raw(const char* key, const size_t key_len)
{
	if (key) {
		ConfigData* k = new ConfigData(CONFIG_T_KEY, key, key_len);
		add_key(k);
	}
}

/**
 * Method add_value(value) will add new value to the last key in Config list.
 */
void ConfigData::add_value(const ConfigData* value)
{
	if (!value) {
		return;
	}
	if (CONFIG_T_KEY != last_head->last_key->type) {
		return;
	}
	if (last_head->last_key->value != NULL) {
		return;
	}
	last_head->last_key->value = (ConfigData*) value;
}

/**
 * Method add_value(value) will add new value to the last key in Config list.
 */
void ConfigData::add_value_raw(const char* value, const size_t value_len)
{
	if (value) {
		ConfigData* v = new ConfigData(CONFIG_T_VALUE, value, value_len);
		add_value(v);
	}
}

/**
 * Method add_misc(misc) will add a non-key and non-header object, i.e.:
 * comment, to Config list.
 */
void ConfigData::add_misc(const ConfigData* misc)
{
	if (misc) {
		last_head->last_key->next_key= (ConfigData*) (misc);
		last_head->last_key		= (ConfigData*) misc;
	}
}

/**
 * Method add_misc(misc, mist_len) will add a non-key and non-header object,
 * i.e.: comment, to Config list.
 */
void ConfigData::add_misc_raw(const char* misc, const size_t misc_len)
{
	if (misc) {
		ConfigData* m = new ConfigData(CONFIG_T_MISC, misc, misc_len);
		add_misc(m);
	}
}

/**
 * Method chars() will dump content of ConfigData object to standard output,
 * including the next header, key, and value.
 */
const char* ConfigData::chars()
{
	Buffer o;
	ConfigData* h = this;
	ConfigData* k = NULL;

	o.append_raw("{\n");

	while (h) {
		if (h != this) {
			o.appendc(',');
		}

		o.append_fmt("\t\"%s\":{\n", h->_v);

		k = h->next_key;
		while (k) {
			if (k != h->next_key) {
				o.append_raw("\t,");
			} else {
				o.appendc('\t');
			}

			if (CONFIG_T_KEY == k->type) {
				o.append_fmt("\t\"%s\": \"%s\"\n"
					, k->v()
					, k->value ? k->value->v() : "");
			} else {
				o.append_fmt("\t\"%s\"\n", k->v());
			}
			k = k->next_key;
		}
		h = h->next_head;

		o.append_raw("\t}\n");
	}

	o.appendc('}');

	__str = o.detach();

	return __str;
}

} // namespace::vos

// vi: ts=8 sw=8 tw=80:
