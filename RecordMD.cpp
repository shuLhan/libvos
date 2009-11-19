/**
 * Copyright (C) 2009 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
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
	_name(NULL),
	_date_format(NULL),
	_fltr_v(NULL),
	_next(NULL)
{}

RecordMD::~RecordMD()
{
	if (_name)
		delete _name;
	if (_date_format)
		delete _date_format;
	if (_fltr_v)
		delete _fltr_v;
	if (_next)
		delete _next;
}

void RecordMD::dump()
{
	RecordMD *p = this;

	while (p) {
		printf("'%c' : %s : '%c' : %3d : %3d | ", 
			p->_left_q, p->_name->_v, p->_right_q, p->_start_p,
			p->_end_p);

		switch (p->_sep) {
		case '\t':
			printf("'\\t'");
			break;
		case '\n':
			printf("'\\n'");
			break;
		case '\v':
			printf("'\\v'");
			break;
		case '\r':
			printf("'\\r'");
			break;
		case '\f':
			printf("'\\f'");
			break;
		case '\b':
			printf("'\\b'");
			break;
		default:
			printf("'%c'", p->_sep);
		}

		printf(" : %2d\n", p->_type);

		p = p->_next;
	}
}

void RecordMD::ADD(RecordMD **rmd, RecordMD *md)
{
	if (! (*rmd)) {
		(*rmd)		= md;
		(*rmd)->_n_md	= 1;
	} else {
		RecordMD *p = (*rmd);
		while (p->_next)
			p = p->_next;

		p->_next = md;
		(*rmd)->_n_md++;
	}
}

/**
 * @desc: initialize meta data using field declaration in 'meta'.
 *
 * field format:
 *
 *	'<char>':name:'<char>':[start-pos]:[end-pos | '<char>']:[type]
 *
 *	[]	: optional.
 *	<char>	: any single character, in c-style for escape char.
 *
 * @param:
 *	> meta	: formatted string of field declaration.
 *
 * @return:
 *	< !NULL	: success.
 *	< NULL	: fail.
 */
RecordMD *RecordMD::INIT(const char *meta)
{
	if (! meta)
		return 0;

	int		i		= 0;
	int		todo		= MD_START;
	int		todo_next	= 0;
	int		len		= strlen(meta);
	Buffer		v;
	RecordMD	*rmd		= NULL;
	RecordMD	*md		= NULL;

	while (todo != MD_DONE) {
		while (i < len && isspace(meta[i])) {
			++i;
		}
		if (i >= len) {
			if (todo_next == MD_START || todo_next == MD_END_P
			||  todo_next == MD_TYPE)
				break;
			else
				goto err;
		}

		switch (todo) {
		case MD_START:
			md	= new RecordMD();
			todo	= MD_LEFT_Q;
			RecordMD::ADD(&rmd, md);
			break;

		case MD_META_SEP:
			switch (meta[i]) {
			case ':':
				todo = todo_next;
				break;
			case ',':
				todo = MD_START;
				break;
			default:
				if (todo_next == MD_TYPE)
					todo = MD_START;
				else
					goto err;
			}

			++i;
			break;

		case MD_LEFT_Q:
			switch (meta[i]) {
			case '\'':
				++i;
				if (i >= len)
					goto err;

				if (meta[i] != '\\') {
					md->_left_q = meta[i];
				} else {
					++i;
					if (i >= len)
						goto err;

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
				if (i >= len)
					goto err;

				if (meta[i] != '\'')
					goto err;

				++i;
				if (i >= len)
					goto err;

				todo		= MD_META_SEP;
				todo_next	= MD_NAME;
				break;
			case ':':
				todo = MD_NAME;
				++i;
				if (i >= len)
					goto err;
				break;
			default:
				goto err;
			}
			break;

		case MD_NAME:
			md->_name = new Buffer();
			while (meta[i] != ':') {
				md->_name->appendc(meta[i]);
				++i;
				if (i >= len)
					goto err;
			}
			++i;
			if (i >= len)
				goto err;

			todo = MD_RIGHT_Q;
			break;

		case MD_RIGHT_Q:
			switch (meta[i]) {
			case '\'':
				++i;
				if (i >= len)
					goto err;

				if (meta[i] != '\\') {
					md->_right_q = meta[i];
				} else {
					++i;
					if (i >= len)
						goto err;

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
				if (i >= len)
					goto err;

				if (meta[i] != '\'')
					goto err;

				++i;
				if (i >= len)
					goto err;

				todo		= MD_META_SEP;
				todo_next	= MD_START_P;
				break;
			case ':':
				todo = MD_START_P;

				++i;
				if (i >= len)
					goto err;
				break;
			default:
				goto err;
			}
			break;

		case MD_START_P:
			while (isdigit(meta[i])) {
				v.appendc(meta[i]);
				++i;
				if (i >= len)
					goto err;
			}

			md->_start_p = strtol(v._v, 0, 0);
			v.reset();

			todo		= MD_META_SEP;
			todo_next	= MD_END_P;
			break;

		case MD_END_P:
			if (isdigit(meta[i])) {
				while (isdigit(meta[i])) {
					v.appendc(meta[i]);
					++i;
					if (i >= len)
						goto err;
				}

				md->_end_p = strtol(v._v, 0, 0);
				v.reset();
			} else if (meta[i] == '\'') {
				++i;
				if (i >= len)
					goto err;

				if (meta[i] == '\\') {
					++i;
					if (i + 1 >= len)
						goto err;

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
						if (i >= len)
							goto err;
					}
				} else {
					md->_sep = meta[i];

					++i;
					if (i >= len)
						goto err;
				}
			
				if (meta[i] != '\'')
					goto err;

				++i;
			}
			todo		= MD_META_SEP;
			todo_next	= MD_TYPE;
			break;

		case MD_TYPE:
			while (meta[i] != ',' && !isspace(meta[i])) {
				v.appendc(meta[i]);
				++i;
				if (i >= len)
					goto err;
			}

			if (v._i) {
				if (v.like("STRING") == 0) {
					md->_type = RMD_T_STRING;
				} else if (v.like("NUMBER") == 0) {
					md->_type = RMD_T_NUMBER;
				} else if (v.like("DATE") == 0) {
					md->_type = RMD_T_DATE;
				} else if (v.like("BLOB") == 0) {
					md->_type = RMD_T_BLOB;
				} else {
					md->_type = RMD_T_STRING;
				}
			} else {
				md->_type = RMD_T_STRING;
			}

			v.reset();
			todo		= MD_META_SEP;
			todo_next	= MD_START;
			break;
		}
	}

	return rmd;
err:
	fprintf(stderr, "invalid field meta data : %s\n", &meta[i]);
	fprintf(stderr, "  at position %d,\n", i);
	fprintf(stderr, "  at character '%c'.\n", meta[i]);
	delete rmd;
	return NULL;
}

RecordMD *RecordMD::INIT_FROM_FILE(const char *fmeta)
{
	File f;

	f.open_ro(fmeta);
	f.resize(f.get_size());
	f.read();
	return RecordMD::INIT(f._v);
}

} /* namespace::vos */
