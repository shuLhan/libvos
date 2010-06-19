/*
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "Config.hpp"

namespace vos {

enum _cfg_parsing_stt {
	P_CFG_DONE	= 0,
	P_CFG_START,
	P_CFG_KEY,
	P_CFG_VALUE
};

/**
 * @method	: Config::Config
 * @desc	: Config object constructor.
 */
Config::Config() :
	_data(NULL)
{}

/**
 * @method	: Config::~Config
 * @desc	: Config object destructor.
 */
Config::~Config()
{
	if (_data) {
		delete _data;
	}
}

/**
 * @method	: Config::dump
 * @desc	: dump content of Config object to standard output.
 */
void Config::dump()
{
	_data->dump();
}

/**
 * @method	: Config::load
 * @param	:
 *	> ini	: a name of configuration file, with or without leading path.
 * @return	:
 *	< 0	: success, or 'ini' is nil.
 *	< <0	: fail.
 * @desc	: open config file and load all key and values.
 */
int Config::load(const char *ini)
{
	register int s;

	if (!ini)
		return 0;

	s = ConfigData::INIT(&_data, CONFIG_T_HEAD, CONFIG_ROOT);
	if (s < 0)
		return s;

	s = File::init(File::DFLT_BUFFER_SIZE);
	if (s < 0)
		return s;

	s = open_ro(ini);
	if (s < 0)
		return s;

	s = parsing();

	return s;
}

/**
 * @method	: Config::save
 * @return	:
 *	> 0	: success.
 *	> <0	: fail.
 * @desc	: save all heads, keys, and values to file.
 */
int Config::save()
{
	register int	s;
	Buffer		ini;

	s = ini.init(&_name);
	if (s < 0)
		return s;

	close();

	s = save_as(ini._v, CONFIG_SAVE_WITH_COMMENT);

	return s;
}

/**
 * @method	: Config::save_as
 * @param	:
 *	> ini	: an output filename, with or without leading path.
 *	> mode	: save with (1) or without comment (0).
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: save all heads, keys, and values to a new 'ini' file.
 */
int Config::save_as(const char *ini, const int mode)
{
	int		s;
	File		fini;
	ConfigData	*phead	= _data;
	ConfigData	*pkey	= NULL;

	if (!ini)
		return 0;

	s = fini.init(File::DFLT_BUFFER_SIZE);
	if (s < 0)
		return s;

	s = fini.open_wo(ini);
	if (s < 0)
		return s;

	while (phead) {
		if (phead->like_raw(CONFIG_ROOT) != 0) {
			s = fini.writes("[%s]\n", phead->_v);
			if (s < 0)
				goto err;
		}

		pkey = phead->_next_key;
		while (pkey) {
			if (CONFIG_T_KEY == pkey->_t) {
				s = fini.writes("\t%s = %s\n",
						pkey->_v,
						pkey->_value ?
						pkey->_value->_v : "");
				if (s < 0) {
					goto err;
				}
			} else {
				if (CONFIG_SAVE_WITH_COMMENT == mode) {
					s = fini.writes("%s\n", pkey->_v);
					if (s < 0) {
						goto err;
					}
				}
			}
			pkey = pkey->_next_key;
		}
		phead = phead->_next_head;
	}
	s = 0;
err:
	fini.close();

	return s;
}

/**
 * @method	: Config::close
 * @desc	: close config file and release all data.
 */
void Config::close()
{
	File::close();
	if (_data) {
		delete _data;
		_data = NULL;
	}
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
const char * Config::get(const char *head, const char *key, const char *dflt)
{
	ConfigData *h = _data;
	ConfigData *k = NULL;

	while (h) {
		if (h->like_raw(head) == 0) {
			k = h;
			while (k) {
				if (CONFIG_T_KEY == k->_t) {
					if (k->like_raw(key) == 0)
						return k->_value->_v;
				}

				k = k->_next_key;
			}
		}

		h = h->_next_head;
	}

	return dflt;
}

const char* Config::get(const char *head, const char *key)
{
	return get(head, key, NULL);
}

const char* Config::get(const char *key)
{
	return get(CONFIG_ROOT, key, NULL);
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
 */
long int Config::get_number(const char *head, const char *key, const int dflt)
{
	long int	n;
	const char	*v = get(head, key, NULL);

	if (!v) {
		return dflt;
	}

	n = strtol(v, 0, 0);

	return n;
}

long int Config::get_number(const char *head, const char *key)
{
	return get_number(head, key, 0);
}

long int Config::get_number(const char *key)
{
	return get_number(CONFIG_ROOT, key, 0);
}

/**
 * @method	: Config::set
 * @param	:
 *	> head	: a name of head, where the key is resided.
 *	> key	: a name of key.
 *	> value	: a new value for key.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: set a 'key' value, where the head is 'head', to 'value'.
 */
int Config::set(const char *head, const char *key, const char *value)
{
	int		s;
	ConfigData	*h = _data;
	ConfigData	*k = NULL;

	if (!head || !key || !value)
		return 0;

	while (h) {
		if (h->like_raw(head) == 0) {
			k = h;
			while (k) {
				if (k->like_raw(key) == 0) {
					s = k->_value->copy_raw(value);
					return s;
				}
				k = k->_next_key;
			}

			/* add key:value to config list, if not found */
			s = ConfigData::INIT(&k, CONFIG_T_KEY, key);
			if (s < 0)
				return s;

			s = ConfigData::INIT(&k->_value, CONFIG_T_VALUE, value);
			if (s < 0)
				return s;

			h->_last_key->_next_key	= k;
			h->_last_key		= k;
			return 0;
		}
		h = h->_next_head;
	}

	s = ConfigData::INIT(&h, CONFIG_T_HEAD, head);
	if (s < 0)
		return s;

	_data->add_head(h);

	s = ConfigData::INIT(&k, CONFIG_T_KEY, key);
	if (s < 0)
		return s;

	_data->add_key(k);

	k = NULL;
	s = ConfigData::INIT(&k, CONFIG_T_VALUE, value);
	if (s < 0)
		return s;

	s = _data->add_value(k);

	return s;
}

/**
 * @method	: Config::add
 * @param	:
 *	> head	: a name of head, where the key is resided.
 *	> key	: a name of key.
 *	> value	: a new value for key.
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	:
 *	add a new 'head', or a new 'key' with 'value', to Config data.
 */
void Config::add(const char *head, const char *key, const char *value)
{
	set(head, key, value);
}

/**
 * @method	: Config::parsing
 * @return	:
 *	< 0	: success.
 *	< <0	: fail.
 * @desc	: inline, parsing content of config file.
 */
inline int Config::parsing()
{
	int	s	= 0;
	int	todo	= P_CFG_START;
	int	line_i	= 0;
	int	i	= 0;
	int	i_str	= 0;
	int	l	= (int) get_size();
	int	_e_row	= 1;
	int	_e_col	= 0;
	Buffer	b;

	s = b.init(NULL);
	if (s != 0) {
		return s;
	}

	resize(l);
	read();

	while (i < l) {
		while (i < l && isspace(_v[i])) {
			if (_v[i] == _eol) {
				++_e_row;
				line_i = i;
			}
			++i;
		}

		if (i >= l) {
			break;
		}

		if (_v[i] == CFG_CH_COMMENT || _v[i] == CFG_CH_COMMENT2) {
			i_str = i;
			while (i < l &&_v[i] != _eol)
				++i;

			line_i = i;
			++i;
			++_e_row;

			s = b.append_raw(&_v[i_str], i - i_str);
			if (s < 0)
				return s;

			b.trim();

			s = _data->add_misc_raw(b._v);
			if (s < 0)
				return s;

			b.reset();

			continue;
		}

		switch (todo) {
		case P_CFG_START:
			if (_v[i] != CFG_CH_HEAD_OPEN) {
				todo = P_CFG_KEY;
				continue;
			}

			++i;
			i_str = i;
			while (i < l && _v[i] != CFG_CH_HEAD_CLOSE
			&& _v[i] != _eol) {
				++i;
			}

			if (i >= l || i_str == i
			|| _v[i] != CFG_CH_HEAD_CLOSE) {
				_e_col = i - line_i;
				goto bad_cfg;
			}

			s = b.append_raw(&_v[i_str], i - i_str);
			if (s < 0)
				return s;

			b.trim();
			/* empty ? */
			if (b._i == 0) {
				_e_col = i - line_i;
				goto bad_cfg;
			}

			s = _data->add_head_raw(b._v);
			if (s < 0) {
				return s;
			}

			b.reset();

			++i;
			todo = P_CFG_KEY;
			break;

		case P_CFG_KEY:
			if (_v[i] == CFG_CH_HEAD_OPEN) {
				todo = P_CFG_START;
				continue;
			}

			i_str = i;
			while (i < l
			&& _v[i] != CFG_CH_KEY_SEP
			&& _v[i] != _eol) {
				++i;
			}
			if (i >= l || _v[i] == _eol) {
				_e_col = i - line_i;
				goto bad_cfg;
			}

			s = b.append_raw(&_v[i_str], i - i_str);
			if (s < 0) {
				return s;
			}

			b.trim();
			/* empty ? */
			if (b._i == 0) {
				_e_col = i - line_i;
				goto bad_cfg;
			}

			s = _data->add_key_raw(b._v);
			if (s < 0) {
				return s;
			}

			b.reset();

			++i;
			todo = P_CFG_VALUE;
			/* no break, keep it flow */

		case P_CFG_VALUE:
			i_str = i;
			while (i < l) {
				if (_v[i] == _eol) {
					++_e_row;
					break;
				}
				++i;
			}

			s = b.append_raw(&_v[i_str], i - i_str);
			if (s < 0) {
				return s;
			}

			b.trim();
			if (b._i == 0) {
				_e_col = i - line_i;
				goto bad_cfg;
			}

			s = _data->add_value_raw(b._v);
			if (s < 0) {
				return s;
			}

			b.reset();

			line_i = i;
			++i;

			todo = P_CFG_KEY;
			break;
		}
	}

	return 0;

bad_cfg:
	fprintf(stderr, " [CONFIG-ERROR] line %d: invalid config format.\n",
			_e_row);

	return -1;
}

} /* namespace::vos */
