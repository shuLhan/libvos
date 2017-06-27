//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LIBVOS_CONFIG_HH
#define _LIBVOS_CONFIG_HH 1

#include "ListBuffer.hh"
#include "File.hh"
#include "ConfigData.hh"

namespace vos {

extern Error ErrConfigFormat;

enum _cfg_ch {
	CONFIG_CH_COMMENT	= '#'
,	CONFIG_CH_COMMENT2	= ';'
,	CONFIG_CH_CONT		= '\\'
,	CONFIG_CH_HEAD_OPEN	= '['
,	CONFIG_CH_HEAD_CLOSE	= ']'
,	CONFIG_CH_KEY_SEP	= '='
};

enum _cfg_save_mode {
	CONFIG_SAVE_WOUT_COMMENT	= 0
,	CONFIG_SAVE_WITH_COMMENT	= 1
};

#define	CONFIG_ROOT	"__CONFIG__"

/**
 * Class Config represents module for reading config file in INI format.
 *
 * Field _data contains list of config headers, keys, and values.
 */
class Config : public File {
public:
	static const char* __CNAME;

	Config();
	~Config();

	Error load(const char* ini);
	Error save(const char* ini, const int mode = CONFIG_SAVE_WOUT_COMMENT);
	void close();

	const char* get(const char* head, const char* key
				, const char* dflt = NULL);
	long int get_number(const char* head, const char* key
				, const int dflt = 0);

	int set(const char* head, const char* key, const char* value);

	int add(const char* head, const char* key, const char* value)
	{
		return set(head, key, value);
	}

	void add_comment(const char* comment);

	const char* chars();

protected:
	ConfigData _data;

private:
	Config(const Config&);
	void operator=(const Config&);

	Error parsing();
};

} /* namespace::vos */
#endif

// vi: ts=8 sw=8 tw=80:
