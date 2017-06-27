//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Config.hh"

namespace vos {

Error ErrConfigFormat("Config: invalid format");

const char* Config::__CNAME = "Config";

enum _cfg_parsing_stt {
	P_CFG_DONE	= 0
,	P_CFG_START
,	P_CFG_KEY
,	P_CFG_VALUE
};

/**
 * Method Config() will create and initialize Config object by creating the
 * "root" header.
 */
Config::Config()
: File()
, _data(CONFIG_T_HEAD, CONFIG_ROOT)
{}

/**
 * Method ~Config() will destroy and release object to memory.
 */
Config::~Config()
{}

/**
 * Method load(ini) will open config file `ini` and load all key and values.
 *
 * On success it will return NULL, otherwise it will return,
 *
 * - ErrFileNotFound if `ini` path is not point to valid file.
 * - ErrConfigFormat if content of `ini` file is not valid.
 * - ErrOutOfMemory if no memory left.
 */
Error Config::load(const char* ini)
{
	close();

	Error err = open_ro(ini);
	if (err != NULL) {
		return err;
	}

	return parsing();
}

/**
 * Method save(ini,mode) will save all config values to `ini` file.
 * If mode is CONFIG_SAVE_WOUT_COMMENT, all comment will not be saved.
 * If mode is CONFIG_SAVE_WITH_COMMENT, all comment will be saved.
 *
 * On success it will return NULL.
 *
 * On failure it will return,
 * - ErrFileNameEmpty: if config filename is empty and `ini` file is empty.
 */
Error Config::save(const char* ini, enum config_save_mode mode)
{
	File fini;
	ConfigData* phead = &_data;
	ConfigData* pkey = NULL;
	Error err;

	if (ini) {
		err = fini.open_wt(ini);
	} else {
		if (_name.is_empty()) {
			return ErrFileNameEmpty;
		}
		err = fini.open_wt(_name.v());
	}
	if (err != NULL) {
		return err;
	}

	while (phead) {
		if (phead->like_raw(CONFIG_ROOT) != 0) {
			err = fini.writef("[%s]\n", phead->v());
			if (err != NULL) {
				return err;
			}
		}

		pkey = phead->next_key;
		while (pkey) {
			if (CONFIG_T_KEY == pkey->type) {
				err = fini.writef("\t%s = %s\n",
						pkey->v(),
						pkey->value ?
						pkey->value->v() : "");
				if (err != NULL) {
					return err;
				}
			} else {
				if (CONFIG_SAVE_WITH_COMMENT == mode) {
					err = fini.writef("%s\n", pkey->v());
					if (err != NULL) {
						return err;
					}
				}
			}
			pkey = pkey->next_key;
		}
		phead = phead->next_head;
	}
	return NULL;
}

/**
 * Method close() will release all data and close the config file.
 */
void Config::close()
{
	File::close();

	if (_data.value) {
		delete _data.value;
		_data.value = NULL;
	}
	if (_data.next_head) {
		delete _data.next_head;
		_data.next_head = NULL;
	}
	if (_data.next_key) {
		delete _data.next_key;
		_data.next_key = NULL;
	}
	_data.last_head = &_data;
	_data.last_key = &_data;
}

/**
 * Method get(head,key,dflt) will return a value that is associated with head
 * and key in config file.
 *
 * head is header of configuration, where key will reside.
 * key is key that will be searched.
 * dflt as default value to be returned if head or key is not found in config
 * file.
 *
 * On success it will return non empty string.
 * On fail, where head or key not found, it will return NULL.
 */
const char* Config::get(const char* head, const char* key, const char* dflt)
{
	if (!head || !key) {
		return dflt;
	}

	ConfigData* h = &_data;
	ConfigData* k = NULL;

	while (h) {
		if (h->like_raw(head) != 0) {
			h = h->next_head;
			continue;
		}

		k = h;
		while (k) {
			if (CONFIG_T_KEY != k->type) {
				k = k->next_key;
				continue;
			}

			if (k->like_raw(key) != 0) {
				k = k->next_key;
				continue;
			}

			if (k->value) {
				return k->value->v();
			}
			return NULL;
		}
	}

	return dflt;
}

/**
 * Method get_number(head,key,dflt) will return a number representation of
 * config value in with 'head' and 'key'.
 */
long int Config::get_number(const char* head, const char* key, const int dflt)
{
	const char *v = get(head, key, NULL);

	if (!v) {
		return dflt;
	}

	return strtol(v, 0, 0);
}

/**
 * Method set(head,key,value) will set a `key` value inside `head` to `value`.
 *
 * If head or key is not found, then it will be added to config data.
 *
 * If value replacement occurred it will return 0.
 * If head or key is not found, then it will return 1.
 */
int Config::set(const char* head, const char* key, const char* value)
{
	ConfigData* h = &_data;
	ConfigData* k = NULL;
	Error err;

	if (!head || !key || !value) {
		return 0;
	}

	while (h) {
		if (h->like_raw(head) != 0) {
			h = h->next_head;
			continue;
		}

		k = h;
		while (k) {
			if (k->like_raw(key) == 0) {
				k->value->copy_raw(value);
				return 0;
			}
			k = k->next_key;
		}

		k = new ConfigData(CONFIG_T_KEY, key);

		k->value = new ConfigData(CONFIG_T_VALUE, value);

		h->last_key->next_key	= k;
		h->last_key		= k;
		return 1;
	}

	_data.add_head_raw(head);
	_data.add_key_raw(key);
	_data.add_value_raw(value);

	return 1;
}

/**
 * Method add_comment(comment) will add comment to Config object.
 */
void Config::add_comment(const char* comment)
{
	size_t len = 0;
	const char*	raw = NULL;

	if (!comment) {
		raw = ";\n";
		len = 2;
	} else {
		len = strlen(comment);
		if (len == 0) {
			raw = ";\n";
			len = 2;
		} else {
			raw = comment;
		}
	}
	_data.add_misc_raw(raw, len);
}

/**
 * Method parsing() will parsing the content of config file.
 *
 * On success it will return NULL, otherwise it will return error.
 */
Error Config::parsing()
{
	Buffer line;
	char c;

	Error err = get_line(&line);

	while (err == NULL) {
		line.trim();

		if (line.is_empty()) {
			err = get_line(&line);
			continue;
		}

		switch (line.char_at(0)) {
		case CONFIG_CH_COMMENT:
		case CONFIG_CH_COMMENT2:
			_data.add_misc_raw(line.v(), line.len());
			break;

		case CONFIG_CH_HEAD_OPEN:
			c = line.char_at(line.len() - 1);
			if (c != CONFIG_CH_HEAD_CLOSE) {
				return ErrConfigFormat.with(line.v(), line.len());
			}
			_data.add_head_raw(line.v(1), line.len() - 2);

			break;

		default:
			List* kv = SPLIT_BY_CHAR(&line, CONFIG_CH_KEY_SEP, 1);

			if (kv == NULL) {
				return ErrConfigFormat.with(line.v(), line.len());
			}

			Buffer* key = (Buffer*) kv->at(0);
			_data.add_key_raw(key->v(), key->len());

			if (kv->size() == 2) {
				Buffer* value = (Buffer*) kv->at(1);
				_data.add_value_raw(value->v(), value->len());
			}

			delete kv;
			break;
		}

		err = get_line(&line);
	}
	if (err != ErrFileEnd) {
		return err;
	}

	return NULL;
}

/**
 * Method chars() will return the JSON representation of config object.
 */
const char* Config::chars()
{
	return _data.chars();
}

} // namespace::vos
// vi: ts=8 sw=8 tw=80:
