//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Config.hh"

namespace vos {

enum _cfg_parsing_stt {
	P_CFG_DONE	= 0
,	P_CFG_START
,	P_CFG_KEY
,	P_CFG_VALUE
};

const char* Config::__cname = "Config";

/**
 * @method	: Config::Config
 * @desc	: Config object constructor.
 */
Config::Config() : File()
,	_data()
{
	_data.init(CONFIG_T_HEAD, CONFIG_ROOT);
}

/**
 * @method	: Config::~Config
 * @desc	: Config object destructor.
 */
Config::~Config()
{
	if (_data._next_head) {
		delete _data._next_head;
		_data._next_head = NULL;
	}
	if (_data._next_key) {
		delete _data._next_key;
		_data._next_key = NULL;
	}
}

/**
 * @method	: Config::load
 * @param	:
 *	> ini	: a name of configuration file, with or without leading path.
 * @return	:
 *	< 0	: success, or 'ini' is nil.
 *	< -1	: fail.
 * @desc	: open config file and load all key and values.
 */
int Config::load(const char* ini)
{
	if (!ini) {
		return 0;
	}

	close();

	int s = open_ro(ini);
	if (s < 0) {
		return -1;
	}

	s = parsing();

	return s;
}

/**
 * @method	: Config::save
 * @return	:
 *	> 0	: success.
 *	> -1	: fail.
 * @desc	: save all heads, keys, and values to file.
 */
int Config::save()
{
	if (_name.is_empty()) {
		printf("[%s] save: filename is empty!\n", __cname);
		return -1;
	}

	Buffer ini;

	int s = ini.copy(&_name);
	if (s < 0) {
		return -1;
	}

	close();

	s = save_as(ini.v(), CONFIG_SAVE_WITH_COMMENT);

	return s;
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
int Config::save_as(const char* ini, const int mode)
{
	ssize_t s = 0;
	File		fini;
	ConfigData*	phead	= &_data;
	ConfigData*	pkey	= NULL;

	if (!ini) {
		printf("[%s] save_as: filename is empty!\n", __cname);
		return -1;
	}

	s = fini.open_wo(ini);
	if (s < 0) {
		return -1;
	}

	while (phead) {
		if (phead->like_raw(CONFIG_ROOT) != 0) {
			s = fini.writes("[%s]\n", phead->chars());
			if (s < 0) {
				return -1;
			}
		}

		pkey = phead->_next_key;
		while (pkey) {
			if (CONFIG_T_KEY == pkey->_t) {
				s = fini.writes("\t%s = %s\n",
						pkey->chars(),
						pkey->_value ?
						pkey->_value->v() : "");
				if (s < 0) {
					return -1;
				}
			} else {
				if (CONFIG_SAVE_WITH_COMMENT == mode) {
					s = fini.writes("%s\n", pkey->v());
					if (s < 0) {
						return -1;
					}
				}
			}
			pkey = pkey->_next_key;
		}
		phead = phead->_next_head;
	}
	return 0;
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

	if (_data._value) {
		delete _data._value;
		_data._value = NULL;
	}
	if (_data._next_head) {
		delete _data._next_head;
		_data._next_head = NULL;
	}
	if (_data._next_key) {
		delete _data._next_key;
		_data._next_key = NULL;
	}
	_data._last_head = &_data;
	_data._last_key = &_data;
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
				if (CONFIG_T_KEY == k->_t) {
					if (k->like_raw(key) == 0)
						return k->_value->v();
				}

				k = k->_next_key;
			}
		}

		h = h->_next_head;
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
	int		s;
	ConfigData*	h = &_data;
	ConfigData*	k = NULL;

	if (!head || !key || !value) {
		return 0;
	}

	while (h) {
		if (h->like_raw(head) == 0) {
			k = h;
			while (k) {
				if (k->like_raw(key) == 0) {
					k->_value->copy_raw(value);
					return -1;
				}
				k = k->_next_key;
			}

			/* add key:value to config list, if not found */
			s = ConfigData::INIT(&k, CONFIG_T_KEY, key);
			if (s < 0) {
				return -1;
			}

			s = ConfigData::INIT(&k->_value, CONFIG_T_VALUE, value);
			if (s < 0) {
				return -1;
			}

			h->_last_key->_next_key	= k;
			h->_last_key		= k;
			return 0;
		}
		h = h->_next_head;
	}

	s = ConfigData::INIT(&h, CONFIG_T_HEAD, head);
	if (s < 0) {
		return -1;
	}

	_data.add_head(h);

	s = ConfigData::INIT(&k, CONFIG_T_KEY, key);
	if (s < 0) {
		return -1;
	}

	_data.add_key(k);

	k = NULL;
	s = ConfigData::INIT(&k, CONFIG_T_VALUE, value);
	if (s < 0) {
		return -1;
	}

	s = _data.add_value(k);

	return s;
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
inline int Config::parsing()
{
	ssize_t s = 0;
	int	todo	= P_CFG_START;
	size_t start = 0;
	size_t end = 0;
	size_t _e_row = 1;
	size_t _e_col = 0;
	Buffer	b;

	s = resize(size_t(get_size()));
	if (s < 0) {
		return -1;
	}

	s = read();
	if (s <= 0) {
		return -1;
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

			s = b.append_raw(&_v[start], _p - start);
			if (s < 0) {
				return -1;
			}

			b.trim();

			s = _data.add_misc_raw(b.v());
			if (s < 0) {
				return -1;
			}

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

			s = b.append_raw(&_v[start], _p - start);
			if (s < 0) {
				return -1;
			}

			b.trim();
			/* empty ? */
			if (b._i == 0) {
				_e_col = _p - end;
				goto bad_cfg;
			}

			s = _data.add_head_raw(b.v(), b._i);
			if (s < 0) {
				return -1;
			}

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

			s = b.append_raw(&_v[start], _p - start);
			if (s < 0) {
				return -1;
			}

			b.trim();
			/* empty ? */
			if (b._i == 0) {
				_e_col = _p - end;
				goto bad_cfg;
			}

			s = _data.add_key_raw(b.v(), b._i);
			if (s < 0) {
				return -1;
			}

			b.reset();

			++_p;
			/* no break, keep it flow */

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
				s = b.append_raw(&_v[start], _p - start);
				if (s < 0) {
					return -1;
				}

				b.trim();
				if (b._i == 0) {
					_e_col = _p - end;
					goto bad_cfg;
				}
			}

			s = _data.add_value_raw(b.v(), b._i);
			if (s < 0) {
				return -1;
			}

			b.reset();

			end = _p;
			++_p;

			todo = P_CFG_KEY;
			break;
		}
	}

	return 0;
bad_cfg:
	fprintf(stderr
	, "[%s] parsing: line '%zu' column '%zu', invalid config format.\n"
	, __cname, _e_row, _e_col);

	return -1;
}

} /* namespace::vos */
// vi: ts=8 sw=8 tw=78:
