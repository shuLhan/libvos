/*
 * Copyright (C) 2014 kilabit.info
 * Author:
 *	- mhd sulhan (ms@kilabit.info)
 */

#include "RecordMD.hpp"

namespace vos {

enum _rmd_parser {
	MD_DONE		= 0,
	MD_START,
	MD_META_SEP,
	MD_LEFT_Q,
	MD_NAME,
	MD_RIGHT_Q,
	MD_START_P,
	MD_END_P,
	MD_TYPE
};

int RecordMD::BLOB_SIZE = sizeof(int);
int RecordMD::DEF_SEP	= ',';

/**
 * @method	: RecordMD::RecordMD
 * @desc	: RecordMD object constructor.
 */
RecordMD::RecordMD() :
	_n_md(0),
	_idx(0),
	_flag(0),
	_type(0),
	_left_q(0),
	_right_q(0),
	_start_p(0),
	_end_p(0),
	_sep(0),
	_fltr_idx(0),
	_fltr_rule(0),
	_fop(NULL),
	_name(),
	_date_format(NULL),
	_fltr_v(NULL),
	_next(NULL)
{}

/**
 * @method	: RecordMD::~RecordMD
 * @desc	: RecordMD object destructor.
 */
RecordMD::~RecordMD()
{
	if (_date_format)
		delete _date_format;
	if (_fltr_v)
		delete _fltr_v;
	if (_next)
		delete _next;
}

/**
 * @method	: RecordMD::dump
 * @desc	: print content of RecordMD object to standard output.
 */
void RecordMD::dump()
{
	Buffer		o;
	RecordMD*	p = this;

	o.append_raw("[vos::RecordMD] dump:\n");
	while (p) {
		o.aprint("'%c' : %s : '%c' : %3d : %3d | " 
			, p->_left_q, p->_name.v(), p->_right_q, p->_start_p
			, p->_end_p);

		switch (p->_sep) {
		case '\t':
			o.append_raw("'\\t'");
			break;
		case '\n':
			o.append_raw("'\\n'");
			break;
		case '\v':
			o.append_raw("'\\v'");
			break;
		case '\r':
			o.append_raw("'\\r'");
			break;
		case '\f':
			o.append_raw("'\\f'");
			break;
		case '\b':
			o.append_raw("'\\b'");
			break;
		default:
			o.aprint("'%c'", p->_sep);
		}

		o.aprint(" : %2d\n", p->_type);

		p = p->_next;
	}
	printf("%s", o.v());
}

/**
 * @method	: RecordMD::ADD
 * @param	:
 *	> rmd	: head of list.
 *	> md	: a new RecordMD object that will be added to list 'rmd'.
 * @desc	: add 'md' object to list 'rmd'.
 */
void RecordMD::ADD(RecordMD **rmd, RecordMD *md)
{
	if (! (*rmd)) {
		(*rmd)		= md;
		(*rmd)->_n_md	= 1;
	} else {
		RecordMD* p = (*rmd);

		while (p->_next) {
			p = p->_next;
		}

		p->_next = md;
		(*rmd)->_n_md++;
	}
}

/**
 * @method	: RecordMD::INIT
 * @param	:
 *	> o	: return value, list of RecordMD object.
 *	> meta	: formatted string of field declaration.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	:
 *	create and initialize meta data using field declaration in 'meta'.
 *
 *	Field format:
 *
 *	['<char>']:name[:['<char>']:[start-pos]:[end-pos | '<char>']:[type]]
 *
 *	name	: name only allow characters: a-z,A-Z,0-9.
 *	[]	: optional field.
 *	<char>	: any single character, except characters: a-z,A-Z,0-9.
 */
int RecordMD::INIT(RecordMD** o, const char* meta)
{
	if (! meta) {
		return 0;
	}

	int		i		= 0;
	int		todo		= MD_START;
	int		todo_next	= 0;
	int		len		= (int) strlen(meta);
	Buffer		v;
	RecordMD*	md		= NULL;

	while (todo != MD_DONE) {
		while (i < len && isspace(meta[i])) {
			++i;
		}
		if (i >= len) {
			if (todo_next == MD_START || todo_next == MD_END_P
			||  todo_next == MD_TYPE) {
				break;
			} else {
				goto err;
			}
		}

		switch (todo) {
		case MD_START:
			md	= new RecordMD();
			todo	= MD_LEFT_Q;
			RecordMD::ADD(o, md);
			break;

		case MD_META_SEP:
			if (i >= len) {
				todo = MD_DONE;
				continue;
			}

			switch (meta[i]) {
			case ':':
				todo = todo_next;
				break;
			case ',':
				if (todo_next > MD_NAME) {
					if (0 == md->_sep) {
						md->_sep = DEF_SEP;
					}
					todo		= MD_START;
				} else if (todo_next == MD_START) {
					todo = todo_next;
				} else {
					goto err;
				}
				break;
			default:
				if (todo_next == MD_TYPE) {
					todo = MD_START;
				} else {
					goto err;
				}
			}

			++i;
			break;

		case MD_LEFT_Q:
			switch (meta[i]) {
			case '\'':
				++i;
				if (i >= len) {
					goto err;
				}
				if (meta[i] != '\\') {
					md->_left_q = meta[i];
				} else {
					++i;
					if (i >= len) {
						goto err;
					}
					switch (meta[i]) {
					case 't':
						md->_left_q = '\t';
						break;
					case 'n':
						md->_left_q = '\n';
						break;
					case 'v':
						md->_left_q = '\v';
						break;
					case 'r':
						md->_left_q = '\r';
						break;
					case 'f':
						md->_left_q = '\f';
						break;
					case 'b':
						md->_left_q = '\b';
						break;
					default:
						md->_left_q = meta[i];
						break;
					}
				}
				++i;
				if (i >= len) {
					goto err;
				}
				if (meta[i] != '\'') {
					goto err;
				}
				++i;
				todo		= MD_META_SEP;
				todo_next	= MD_NAME;
				break;
			case ':':
				todo = MD_NAME;
				++i;
				break;
			default:
				goto err;
			}
			break;

		case MD_NAME:
			while (i < len) {
				if (meta[i] == ':' || meta[i] == ',') {
					break;
				}
				if (isspace(meta[i])) {
					do {
						++i;
					} while (i < len && isspace(meta[i]));
					break;
				}

				md->_name.appendc(meta[i]);
				++i;
			}
			todo		= MD_META_SEP;
			todo_next	= MD_RIGHT_Q;
			break;

		case MD_RIGHT_Q:
			switch (meta[i]) {
			case '\'':
				++i;
				if (i >= len) {
					goto err;
				}
				if (meta[i] != '\\') {
					md->_right_q = meta[i];
				} else {
					++i;
					if (i >= len) {
						goto err;
					}
					switch (meta[i]) {
					case 't':
						md->_right_q = '\t';
						break;
					case 'n':
						md->_right_q = '\n';
						break;
					case 'v':
						md->_right_q = '\v';
						break;
					case 'r':
						md->_right_q = '\r';
						break;
					case 'f':
						md->_right_q = '\f';
						break;
					case 'b':
						md->_right_q = '\b';
						break;
					default:
						md->_right_q = meta[i];
						break;
					}
				}
				++i;
				if (i >= len) {
					goto err;
				}
				if (meta[i] != '\'') {
					goto err;
				}
				++i;
				todo		= MD_META_SEP;
				todo_next	= MD_START_P;
				break;
			case ':':
				todo = MD_START_P;
				++i;
				break;
			case ',':
				md->_sep	= DEF_SEP;
				todo		= MD_START;
				++i;
				break;
			default:
				goto err;
			}
			break;

		case MD_START_P:
			while (i < len && isdigit(meta[i])) {
				v.appendc(meta[i]);
				++i;
			}

			md->_start_p = (int) strtol(v.v(), 0, 0);
			v.reset();

			todo		= MD_META_SEP;
			todo_next	= MD_END_P;
			break;

		case MD_END_P:
			if (isdigit(meta[i])) {
				while (i < len && isdigit(meta[i])) {
					v.appendc(meta[i]);
					++i;
				}

				md->_end_p = (int) strtol(v.v(), 0, 0);
				v.reset();
			} else if (meta[i] == '\'') {
				++i;
				if (i >= len) {
					goto err;
				}
				if (meta[i] == '\\') {
					++i;
					if (i + 1 >= len) {
						goto err;
					}
					if (meta[i] == '\''
					&& meta[i + 1] != '\'') {
						md->_sep = '\\';
					} else {
						switch (meta[i]) {
						case 't':
							md->_sep = '\t';
							break;
						case 'n':
							md->_sep = '\n';
							break;
						case 'v':
							md->_sep = '\v';
							break;
						case 'r':
							md->_sep = '\r';
							break;
						case 'f':
							md->_sep = '\f';
							break;
						case 'b':
							md->_sep = '\b';
							break;
						default:
							md->_sep = meta[i];
							break;
						}

						++i;
						if (i >= len) {
							goto err;
						}
					}
				} else {
					md->_sep = meta[i];

					++i;
					if (i >= len) {
						goto err;
					}
				}
			
				if (meta[i] != '\'') {
					goto err;
				}
				++i;
			} else {
				md->_sep = DEF_SEP;
			}
			todo		= MD_META_SEP;
			todo_next	= MD_TYPE;
			break;

		case MD_TYPE:
			while (i < len && meta[i] != ',' && !isspace(meta[i])) {
				v.appendc(meta[i]);
				++i;
			}

			if (v._i) {
				if (v.like_raw("NUMBER") == 0) {
					md->_type = RMD_T_NUMBER;
				} else if (v.like_raw("DATE") == 0) {
					md->_type = RMD_T_DATE;
				} else if (v.like_raw("BLOB") == 0) {
					md->_type = RMD_T_BLOB;
				} else {
					md->_type = RMD_T_STRING;
				}
				v.reset();
			} else {
				md->_type = RMD_T_STRING;
			}

			todo		= MD_META_SEP;
			todo_next	= MD_START;
			break;
		}
	}

	return 0;
err:
	fprintf(stderr
	, "[vos::RecordMD] INIT: invalid field meta data : %s\n"	\
	  "                at position '%d', at character '%c'.\n"
	, &meta[i], i, meta[i]);

	if ((*o)) {
		delete (*o);
		(*o) = NULL;
	}

	return -1;
}

/**
 * @method	: RecordMD::INIT_FROM_FILE
 * @param	:
 *	> o	: return value, list of RecordMD object.
 *	> fmeta	: a name of file contains meta-data, with or without leading
 *                path.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	:
 *	create and initialize meta-data object 'o' by loading meta-data from
 *	file 'fmeta'.
 */
int RecordMD::INIT_FROM_FILE(RecordMD** o, const char* fmeta)
{
	register int	s;
	File		f;

	s = f.open_ro(fmeta);
	if (s < 0) {
		return -1;
	}

	s = f.resize((int) f.get_size());
	if (s < 0) {
		return -1;
	}

	s = f.read();
	if (s < 0) {
		return -1;
	}

	return RecordMD::INIT(o, f.v());
}

} /* namespace::vos */
