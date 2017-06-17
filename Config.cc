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
	if (ini == NULL) {
		return ErrFileNameEmpty;
	}

	close();

	Error err = open_ro(ini);
	if (err != NULL) {
		return err;
	}

	return parsing();
}

/**
 * @method	: Config::save
 * @return	:
 *	> 0	: success.
 *	> -1	: fail.
 * @desc	: save all heads, keys, and values to file.
 */
Error Config::save()
{
	if (_name.is_empty()) {
		return ErrFileNameEmpty;
	}

	Buffer ini;

	Error err = ini.copy(&_name);
	if (err != NULL) {
		return err;
	}

	close();

	return save_as(ini.v(), CONFIG_SAVE_WITH_COMMENT);
}

/**
 * @method	: Config::save_as
 * @param	:
 *	> ini	: an output filename, with or without leading path.
 *	> mode	: save with (1) or without comment (0).
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: save all heads, keys, and values to a new 'ini' file.
 */
Error Config::save_as(const char* ini, const int mode)
{
	if (!ini) {
		return ErrFileNameEmpty;
	}

	File fini;
	ConfigData* phead = &_data;
	ConfigData* pkey = NULL;

	Error err = fini.open_wo(ini);
	if (err != NULL) {
		return err;
	}

	while (phead) {
		if (phead->like_raw(CONFIG_ROOT) != 0) {
			err = fini.writef("[%s]\n", phead->chars());
			if (err != NULL) {
				return err;
			}
		}

		pkey = phead->next_key;
		while (pkey) {
			if (CONFIG_T_KEY == pkey->type) {
				err = fini.writef("\t%s = %s\n",
						pkey->chars(),
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
 * @method	: Config::dump
 * @desc	: dump content of Config object to standard output.
 */
void Config::dump()
{
	_data.dump();
}

/**
 * @method	: Config::close
 * @desc	: close config file and release all data.
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
 * @method		: Config::get
 * @param		:
 *	> head		: header of configuration, where key will reside.
 *	> key		: key of value that will be searched.
 *	> dflt		: default return value if head or key is not found in
 *                        config file.
 * @return		:
 *	< value		: success, pointer to config value.
 *	< NULL		: fail, no 'head' or 'key' found in config file, and
 *                        'dflt' parameter is NULL too.
 * @desc		:
 *	get config value, based on 'head' and 'key'. If 'head' or 'key' is not
 *	found then return 'dflt' value.
 */
const char* Config::get(const char* head, const char* key, const char* dflt)
{
	if (!head || !key) {
		return dflt;
	}

	ConfigData* h = &_data;
	ConfigData* k = NULL;

	while (h) {
		if (h->like_raw(head) == 0) {
			k = h;
			while (k) {
				if (CONFIG_T_KEY == k->type) {
					if (k->like_raw(key) == 0)
						return k->value->v();
				}

				k = k->next_key;
			}
		}

		h = h->next_head;
	}

	return dflt;
}

/**
 * @method	: Config::get_number
 * @param	:
 *	> head	: header of configuration.
 *	> key	: key of value that will be searched.
 *	> dflt	: default return value if 'head' or 'key' is not found in
 *                config file.
 * @return	:
 *	< int	: a value, converted from string to number.
 * @desc	: get a number representation of config value in section
 * 'head' and had the 'key'.
 */
long int Config::get_number(const char* head, const char* key, const int dflt)
{
	if (!head || !key) {
		return dflt;
	}

	const char *v = get(head, key, NULL);

	if (!v) {
		return dflt;
	}

	return strtol(v, 0, 0);
}

/**
 * @method	: Config::set
 * @param	:
 *	> head	: a name of head, where the key is resided.
 *	> key	: a name of key.
 *	> value	: a new value for key.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: set a 'key' value, where the head is 'head', to 'value'.
 */
int Config::set(const char* head, const char* key, const char* value)
{
	ConfigData*	h = &_data;
	ConfigData*	k = NULL;
	Error err;

	if (!head || !key || !value) {
		return 0;
	}

	while (h) {
		if (h->like_raw(head) == 0) {
			k = h;
			while (k) {
				if (k->like_raw(key) == 0) {
					k->value->copy_raw(value);
					return -1;
				}
				k = k->next_key;
			}

			/* add key:value to config list, if not found */
			k = new ConfigData(CONFIG_T_KEY, key);

			k->value = new ConfigData(CONFIG_T_VALUE, value);
			if (err != NULL) {
				return -1;
			}

			h->last_key->next_key	= k;
			h->last_key		= k;
			return 0;
		}
		h = h->next_head;
	}

	_data.add_head_raw(head);
	_data.add_key_raw(key);
	_data.add_value_raw(value);

	return 0;
}

/**
 * @method		: Config::add_comment
 * @param		:
 *	> comment	: string of comment.
 * @desc		: add comment to Config object.
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
 * @method	: Config::parsing
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: inline, parsing content of config file.
 */
Error Config::parsing()
{
	int todo = P_CFG_START;
	size_t start = 0;
	size_t end = 0;
	size_t _e_row = 1;
	size_t _e_col = 0;
	Buffer b;

	Error err = resize(size_t(size()));
	if (err != NULL) {
		return err;
	}

	err = read();
	if (err != NULL) {
		if (_i == 0) {
			return ErrFileEmpty;
		}
		return err;
	}

	_p = 0;
	while (_p < _i) {
		/* skip white-space at the beginning of line */
		while (_p < _i && isspace(_v[_p])) {
			if (_v[_p] == _eol) {
				++_e_row;
				end = _p;
			}
			++_p;
		}
		if (_p >= _i) {
			break;
		}

		/* read comment, save as MISC type */
		if (_v[_p] == CFG_CH_COMMENT || _v[_p] == CFG_CH_COMMENT2) {
			start = _p;
			while (_p < _i &&_v[_p] != _eol) {
				++_p;
			}

			end = _p;
			++_p;
			++_e_row;

			err = b.append_raw(&_v[start], _p - start);
			if (err != NULL) {
				return err;
			}

			b.trim();

			_data.add_misc_raw(b.v());

			b.reset();

			continue;
		}

		switch (todo) {
		case P_CFG_START:
			if (_v[_p] != CFG_CH_HEAD_OPEN) {
				todo = P_CFG_KEY;
				continue;
			}

			++_p;
			start = _p;
			while (_p < _i && _v[_p] != CFG_CH_HEAD_CLOSE
			&& _v[_p] != _eol) {
				++_p;
			}

			if (_p >= _i || start == _p
			|| _v[_p] != CFG_CH_HEAD_CLOSE) {
				_e_col = _p - end;
				goto bad_cfg;
			}

			err = b.append_raw(&_v[start], _p - start);
			if (err != NULL) {
				return err;
			}

			b.trim();
			/* empty ? */
			if (b.len() == 0) {
				_e_col = _p - end;
				goto bad_cfg;
			}

			_data.add_head_raw(b.v(), b.len());

			b.reset();

			++_p;
			todo = P_CFG_KEY;
			break;

		case P_CFG_KEY:
			if (_v[_p] == CFG_CH_HEAD_OPEN) {
				todo = P_CFG_START;
				continue;
			}

			start = _p;
			while (_p < _i
			&& _v[_p] != CFG_CH_KEY_SEP
			&& _v[_p] != _eol) {
				++_p;
			}
			if (_p >= _i || _v[_p] == _eol) {
				_e_col = _p - end;
				goto bad_cfg;
			}

			err = b.append_raw(&_v[start], _p - start);
			if (err != NULL) {
				return err;
			}

			b.trim();
			/* empty ? */
			if (b.len() == 0) {
				_e_col = _p - end;
				goto bad_cfg;
			}

			_data.add_key_raw(b.v(), b.len());

			b.reset();

			++_p;
			// fallthrough

		case P_CFG_VALUE:
			start = _p;
			while (_p < _i) {
				if (_v[_p] == _eol) {
					++_e_row;
					break;
				}
				++_p;
			}

			if (_p > start) {
				err = b.append_raw(&_v[start], _p - start);
				if (err != NULL) {
					return err;
				}

				b.trim();
				if (b.len() == 0) {
					_e_col = _p - end;
					goto bad_cfg;
				}
			}

			_data.add_value_raw(b.v(), b.len());

			b.reset();

			end = _p;
			++_p;

			todo = P_CFG_KEY;
			break;
		}
	}

	return 0;

bad_cfg:
	Buffer errData;

	errData.append_fmt("parsing: at line '%u' column '%u'", _e_row
		, _e_col);

	return ErrConfigFormat.with(errData.v(), errData.len());
}

} // namespace::vos

// vi: ts=8 sw=8 tw=80:
