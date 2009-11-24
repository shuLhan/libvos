/**
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

Config::Config(const char *ini) :
	_data(new ConfigData(CONFIG_T_HEAD, CFG_HDR))
{
	if (ini)
		load(ini);
}

Config::~Config()
{
	if (_data)
		delete _data;
}

void Config::dump()
{
	_data->dump();
}

int Config::load(const char *ini)
{
	int s;

	if (!ini)
		return 0;

	s = open_ro(ini);
	if (!s)
		parsing();

	return s;
}

void Config::save()
{
	Buffer ini(&_name);
	close();
	save_as(ini._v, CONFIG_SAVE_WITH_COMMENT);
}

/**
 * @return:
 *	< 0	: success.
 *	< !0	: fail.
 */
int Config::save_as(const char *ini, const int save_mode)
{
	int		s;
	File		fini;
	ConfigData	*phead	= _data;
	ConfigData	*pkey	= NULL;

	if (!ini)
		return 0;

	s = fini.open_wo(ini);
	if (s)
		return s;

	while (phead) {
		if (phead->like(CFG_HDR) != 0)
			fini.writes("[%s]\n", phead->_v);

		pkey = phead->_next_key;
		while (pkey) {
			if (CONFIG_T_KEY == pkey->_t) {
				fini.writes("\t%s = %s\n",
						pkey->_v,
						pkey->_value->_v);
			} else {
				if (CONFIG_SAVE_WITH_COMMENT == save_mode)
					fini.writes("%s\n", pkey->_v);
			}

			pkey = pkey->_next_key;
		}

		phead = phead->_next_head;
	}

	fini.close();

	return 0;
}

/**
 * @desc: get config value, based on 'head' and 'key'. If 'head' or 'key' is
 *	not found then return 'dflt' value;
 *
 * @param:
 *	> head	: header of configuration, where key will reside.
 *	> key	: key of value that will be searched.
 *	> dflt	: default return value if head or key is not found in config
 *		file.
 *
 * @return:
 *	< const char *	: success, pointer to config value.
 *	< NULL		: fail, no head or key found in config file, and dflt
 *			parameter is NULL too.
 */
const char * Config::get(const char *head, const char *key, const char *dflt)
{
	ConfigData *h = _data;
	ConfigData *k = NULL;

	while (h) {
		if (h->like(head) == 0) {
			k = h;
			while (k) {
				if (CONFIG_T_KEY == k->_t) {
					if (k->like(key) == 0)
						return k->_value->_v;
				}

				k = k->_next_key;
			}
		}

		h = h->_next_head;
	}

	return dflt;
}

int Config::get_number(const char *head, const char *key, const int dflt)
{
	int		n;
	const char	*v = get(head, key, NULL);

	if (! v)
		return dflt;

	n = strtol(v, 0, 0);

	return n;
}

void Config::set(const char *head, const char *key, const char *value)
{
	ConfigData *h = _data;
	ConfigData *k = NULL;

	if (!head || !key || !value)
		return;

	while (h) {
		if (h->like(head) == 0) {
			k = h;
			while (k) {
				if (k->like(key) == 0) {
					k->_value->copy(value, 0);
					return;
				}
				k = k->_next_key;
			}

			/* add key:value to config list, if not found */
			k = new ConfigData(CONFIG_T_KEY, key);
			k->_value = new ConfigData(CONFIG_T_VALUE, value);

			h->_last_key->_next_key	= k;
			h->_last_key		= k;
			return;
		}
		h = h->_next_head;
	}

	_data->add_head(new ConfigData(CONFIG_T_HEAD, head));
	_data->add_key(new ConfigData(CONFIG_T_KEY, key));
	_data->add_value(new ConfigData(CONFIG_T_VALUE, value));
}

void Config::add(const char *head, const char *key, const char *value)
{
	set(head, key, value);
}

void Config::parsing()
{
	int	s	= P_CFG_START;
	int	lines	= 1;
	int	line_i	= 0;
	int	i	= 0;
	int	i_str	= 0;
	int	l	= get_size();
	Buffer	b;

	resize(l);
	read();

	while (i < l) {
		while (i < l && isspace(_v[i])) {
			if (_v[i] == _eol) {
				++lines;
				line_i = i;
			}
			++i;
		}

		if (i >= l)
			break;

		if (_v[i] == CFG_CH_COMMENT || _v[i] == CFG_CH_COMMENT2) {
			i_str = i;
			while (i < l &&_v[i] != _eol)
				++i;

			line_i = i;
			++i;
			++lines;

			b.append(&_v[i_str], i - i_str);
			b.trim();
			_data->add_misc(b._v);
			b.reset();

			continue;
		}

		switch (s) {
		case P_CFG_START:
			if (_v[i] != CFG_CH_HEAD_OPEN) {
				throw Error(E_CFG_BAD, lines, i - line_i);
			}

			++i;
			i_str = i;
			while (i < l && _v[i] != CFG_CH_HEAD_CLOSE
			&& _v[i] != _eol) {
				++i;
			}

			if (i >= l || i_str == i || _v[i] != CFG_CH_HEAD_CLOSE)
				throw Error(E_CFG_BAD, lines, i - line_i);

			b.append(&_v[i_str], i - i_str);
			b.trim();
			/* empty ? */
			if (b._i == 0)
				throw Error(E_CFG_BAD, lines, i - line_i);

			_data->add_head(b._v);
			b.reset();

			++i;
			s = P_CFG_KEY;
			break;

		case P_CFG_KEY:
			if (_v[i] == CFG_CH_HEAD_OPEN) {
				s = P_CFG_START;
				continue;
			}

			i_str = i;
			while (i < l
			&& _v[i] != CFG_CH_KEY_SEP
			&& _v[i] != _eol) {
				++i;
			}
			if (i >= l || _v[i] == _eol) {
				throw Error(E_CFG_BAD, lines, i - line_i);
			}

			b.append(&_v[i_str], i - i_str);
			b.trim();
			/* empty ? */
			if (b._i == 0) {
				throw Error(E_CFG_BAD, lines, i - line_i);
			}
			_data->add_key(b._v);
			b.reset();

			++i;
			s = P_CFG_VALUE;
			/* no break, keep it flow */

		case P_CFG_VALUE:
			i_str = i;
			while (i < l) {
				if (_v[i] == _eol) {
					++lines;
					break;
				}
				++i;
			}

			b.append(&_v[i_str], i - i_str);
			b.trim();
			if (b._i == 0) {
				throw Error(E_CFG_BAD, lines, i - line_i);
			}
			_data->add_value(b._v);
			b.reset();

			line_i = i;
			++i;

			s = P_CFG_KEY;
			break;
		}
	}
}

} /* namespace::vos */
