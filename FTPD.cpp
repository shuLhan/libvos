/*
 * Copyright (C) 2010 kilabit.org
 * Author:
 *	- m.shulhan (ms@kilabit.org)
 */

#include "FTPD.hpp"

namespace vos {

const char* _FTP_reply_msg[N_REPLY_CODE] =
{
	"150 File status okay; about to open data connection.\r\n"
,	"200 Command okay. %s\r\n"
,	"211-features.\r\n SIZE\r\n MDTM\r\n211 end\r\n"
,	"213 %s\r\n"
,	"215 UNIX Type: L8\r\n"
,	"220 Service ready for new user.\r\n"
,	"221 Service closing control connection.\r\n"
,	"226 Closing data connection, requested file action successful.\r\n"
,	"227 =%s\r\n"
,	"230 User logged in, proceed.\r\n"
,	"250 Requested file action okay, completed.\r\n"
,	"257 \"%s\"\r\n"
,	"331 User name okay, need password.\r\n"
,	"350 Requested file action pending further information.\r\n"
,	"421 Service not available, closing control connection.\r\n"
,	"425 Can't open data connection.\r\n"
,	"450 Requested file action not taken.\r\n"
,	"451 Requested action aborted: local error in processing.\r\n"
,	"501 Syntax error in parameters or arguments.\r\n"
,	"502 Command not implemented.\r\n"
,	"503 Bad sequence of commands.\r\n"
,	"530 Not logged in.\r\n"
,	"550 Requested action not taken: %s\r\n"
,	"553 Requested action not taken: %s\r\n"
};

const char* _FTP_add_reply_msg[N_ADD_REPLY_MSG] =
{
	"File or directory is not exist."
,	"TYPE is always in B[I]NARY / [I]MAGE."
,	"MODE is always in [S]TREAM."
,	"STRU is always in [F]ILE."
};

static FTPD* _ftpd_ = NULL;

FTPD::FTPD() :
	_running(0)
,	_auth_mode(AUTH_NOLOGIN)
,	_fds_max(0)
,	_path()
,	_dir()
,	_fds_all()
,	_fds_read()
,	_all_client(NULL)
,	_users(NULL)
{}

FTPD::~FTPD()
{
	FTPClient *next = NULL;

	while (_all_client) {
		next = _all_client->_next;

		_all_client->_next = NULL;
		_all_client->reply_raw(CODE_421, _FTP_reply_msg[CODE_421], 0);
		delete _all_client;

		_all_client = next;
	}
	if (_users) {
		delete _users;
	}
}

/**
 * @method		: FTPD::init
 * @param		:
 *	> address	: Address to listen for client connection.
 *                        Default is "0.0.0.0".
 *	> port		: Port to listen for clien connection.
 *                        Default is 21.
 *	> path		: Path to be served to client.
 *                        Default to empty or NULL.
 *	> mode		: Is it anonymous server or authentication server.
 *                        Default to no authentication, all user name allowed.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: initialize FTP server environment.
 */
int FTPD::init(const char* address, const int port, const char* path
		, const int auth_mode)
{
	int s;

	srand(time(0));

	_ftpd_		= this;
	_auth_mode	= auth_mode;

	if (path) {
		s = set_path(path);
		if (s < 0) {
			return s;
		}
	}

	s = create_tcp();
	if (s < 0) {
		return s;
	}

	s = bind_listen(address, port);
	if (s < 0) {
		return s;
	}

	FD_ZERO(&_fds_all);
	FD_ZERO(&_fds_read);
	FD_SET(_d, &_fds_all);

	memset(_fcb, 0, N_FTP_CMD * sizeof(_fcb[0]));

	return 0;
}

/**
 * @method	: FTPD::add_user
 * @param	:
 *	> name	: a name for new user.
 *	> pass	: a string for authentication user when login.
 * @return	:
 *	< 1	: fail, user 'name' already exist.
 *	< 0	: success.
 *	< -1	: fail, system error.
 * @desc	: add new user that can login to this server.
 */
int FTPD::add_user(const char* name, const char* pass)
{
	int		s;
	FTPUser*	user	= NULL;

	s = FTPUser::INIT(&user, name, pass);
	if (s < 0) {
		return s;
	}

	s = FTPUser::ADD(&_users, user);
	if (s < 0) {
		return 1;
	}

	return 0;
}

/**
 * @method	: FTPD::set_path
 * @param	:
 *	> path	: path to directory to be served.
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: set and scan directory pointed by 'path' as the root
 * directory served to client.
 */
int FTPD::set_path(const char* path)
{
	int s;

	if (_dir._ls) {
		_dir.close();
	}

	s = _dir.open(path, -1);
	if (s < 0) {
		return s;
	}

	_path.copy(&_dir._name);
	_dir._ls->_name.copy_raw("/");

	return 0;
}

/**
 * @method	: FTPD::set_default_callback
 * @desc	: set callback for each command on this FTP server.
 */
void FTPD::set_default_callback()
{
	set_callback(FTP_ACCEPT, &on_accept);
	set_callback(FTP_CMD_USER, &on_cmd_USER);
	set_callback(FTP_CMD_PASS, &on_cmd_PASS);
	set_callback(FTP_CMD_SYST, &on_cmd_SYST);
	set_callback(FTP_CMD_TYPE, &on_cmd_TYPE);
	set_callback(FTP_CMD_MODE, &on_cmd_MODE);
	set_callback(FTP_CMD_STRU, &on_cmd_MODE);
	set_callback(FTP_CMD_FEAT, &on_cmd_FEAT);
	set_callback(FTP_CMD_SIZE, &on_cmd_SIZE);
	set_callback(FTP_CMD_MDTM, &on_cmd_MDTM);

	set_callback(FTP_CMD_CWD, &on_cmd_CWD);
	set_callback(FTP_CMD_CDUP, &on_cmd_CDUP);
	set_callback(FTP_CMD_PWD, &on_cmd_PWD);

	set_callback(FTP_CMD_PASV, &on_cmd_PASV);
	set_callback(FTP_CMD_LIST, &on_cmd_LIST);
	set_callback(FTP_CMD_NLST, &on_cmd_NLST);
	set_callback(FTP_CMD_RETR, &on_cmd_RETR);
	set_callback(FTP_CMD_STOR, &on_cmd_STOR);

	set_callback(FTP_CMD_DELE, &on_cmd_DELE);
	set_callback(FTP_CMD_RNFR, &on_cmd_RNFR);
	set_callback(FTP_CMD_RNTO, &on_cmd_RNTO);
	set_callback(FTP_CMD_RMD, &on_cmd_RMD);
	set_callback(FTP_CMD_MKD, &on_cmd_MKD);

	set_callback(FTP_CMD_QUIT, &on_cmd_QUIT);
}

/**
 * @method		: FTPD::set_callback
 * @param		:
 *	> type		: type of callback (see _FTP_cmd_idx).
 *	> (*callback)	: pointer to callback function.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		: set a callback based on specific event.
 *	callback function is called after server accepting a new client or
 *	receiving any command from client.
 */
int FTPD::set_callback(const int type, void (*callback)(FTPD*,FTPClient*))
{
	if (type < 0 || type >= N_FTP_CMD) {
		return -1;
	}

	_fcb[type] = (*callback);

	return 0;
}

/**
 * @method		: EXIT
 * @param		:
 *	> signum	: signal number.
 * @desc		: called when program get signal interrupt or quit.
 */
static void EXIT(int signum)
{
	if (signum == SIGINT || signum == SIGQUIT) {
		if (LIBVOS_DEBUG) {
			printf("[LIBVOS::FTPD____] exit\n");
		}
		if (_ftpd_) {
			_ftpd_->_running = 0;
		}
	}
}

/**
 * @method	: FTPD::RUN
 * @return	:
 *	< 0	: success.
 * @desc	: run FTP server.
 */
int FTPD::run()
{
	int		s;
	Socket*		sock		= NULL;
	FTPClient*	my_client	= NULL;

	_fds_max	= _d + 1;
	_running	= 1;

	signal(SIGINT, &EXIT);
	signal(SIGQUIT, &EXIT);

	while (_running) {
		if (LIBVOS_DEBUG) {
			printf("[LIBVOS::FTPD____] waiting for client\n\n");
		}

		_fds_read = _fds_all;
		s = select(_fds_max, &_fds_read, NULL, NULL, NULL);
		if (s <= 0) {
			if (s < 0) {
				goto err;
			}
			break;
		}
		if (!_running) {
			break;
		}
		if (FD_ISSET(_d, &_fds_read)) {
			sock = accept_conn();
			if (sock) {
				my_client = new FTPClient(sock);
				if (my_client) {
					client_add(my_client);
				}
			}
		} else {
			client_process();
		}
	}
	s = 0;
err:
	if (errno == EINTR) {
		s = 0;
	}
	if (s) {
		perror(NULL);
	}
	return s;
}

/**
 * @method	: FTPD::client_process
 * @return	:
 *	< 0	: success.
 *	< -1	: fail.
 * @desc	: check, receive, and process data from user.
 */
void FTPD::client_process()
{
	int		s;
	Socket*		csock	= NULL;
	Socket*		cpsvr	= NULL;
	FTPClient*	c	= _all_client;
	FTPClient*	cnext	= NULL;

	while (c) {
		cnext	= c->_next;
		csock	= c->_sock;
		cpsvr	= c->_psrv;

		if (cpsvr) {
			if (FD_ISSET(cpsvr->_d, &_fds_read)) {
				c->_pclt = cpsvr->accept_conn();
				FD_CLR(cpsvr->_d, &_fds_all);
			}
		}
		if (!FD_ISSET(csock->_d, &_fds_read)) {
			goto next;
		}

		c->reset();

		s = csock->read();
		if (s <= 0) {
			if (s == 0) {
				client_del(c);
			}
			goto next;
		}

		s = c->_cmnd.get(csock);
		if (s < 0) {
			on_cmd_unknown(c);
		} else {
			if (LIBVOS_DEBUG) {
				c->_cmnd.dump();
			}

			if (_auth_mode == AUTH_LOGIN
			&&  c->_conn_stat != FTP_STT_LOGGED_IN
			&& (c->_cmnd._code != FTP_CMD_USER
			&&  c->_cmnd._code != FTP_CMD_PASS
			&&  c->_cmnd._code != FTP_CMD_QUIT
			&&  c->_cmnd._code != FTP_CMD_SYST)) {
				c->reply_raw(CODE_530
					, _FTP_reply_msg[CODE_530], NULL);
			} else if (_fcb[c->_cmnd._code] != NULL) {
				_fcb[c->_cmnd._code](this, c);
			} else {
				on_cmd_unknown(c);
			}
		}
next:
		c = cnext;
	}
}

/**
 * @method	: FTPD::client_add
 * @param	:
 *	> c	: pointer to FTPClient object.
 * @desc	: add client 'c' to list of FTP client.
 */
void FTPD::client_add(FTPClient* c)
{
	FTPClient::ADD(&_all_client, c);

	FD_SET(c->_sock->_d, &_fds_all);
	if (c->_sock->_d >= _fds_max) {
		_fds_max = c->_sock->_d + 1;
	}
	if (_fcb[FTP_ACCEPT]) {
		_fcb[FTP_ACCEPT](this, c);
	}
}

/**
 * @method	: FTPD::client_del
 * @param	:
 *	> c	: pointer to FTPClient object.
 * @desc	: remove client 'c' from list of FTP client.
 */
void FTPD::client_del(FTPClient* c)
{
	if (LIBVOS_DEBUG) {
		printf("[LIBVOS::FTPD____] client '%d' quit.\n", c->_sock->_d);
	}

	FD_CLR(c->_sock->_d, &_fds_all);

	if (c->_sock) {
		remove_client_r(c->_sock);
		delete c->_sock;
		c->_sock = NULL;
	}

	FTPClient::REMOVE(&_all_client, c);
}

/**
 * @param		:
 *	> server	: pointer to server socket, FTPD object.
 *	> cmd_path	: path to be looking for node (minus last-node).
 *	> node		: return value, pointer to DirNode object.
 *	> rpath		: return value, real-path of 'cmd_path' in the system.
 *	> dirname	: return value, last-node name in 'cmd_path'.
 * @return		:
 *	< 0		: success.
 *	< -1		: fail.
 * @desc		:
 *
 * get pointer to DirNode object that point to 'cmd_path' minus last-node.
 *
 * In example: if 'cmd_path' is '../b/c', then 'rpath' will become '/a/b/c'
 * and 'node' will point to DirNode object to path '/a/b' and 'dirname' will
 * contain only 'c'.
 */
int FTPD::get_path_node(FTPD* s, FTPClient* c, Buffer* cmd_path
			, DirNode** node, Buffer* rpath, Buffer* dirname)
{
	if (!cmd_path || (cmd_path && cmd_path->is_empty())) {
		return CODE_501;
	}

	int	i;
	int	tmp_i;
	Buffer	path;

	if (cmd_path->_v[0] == '/') {
		path.concat(s->_path._v, cmd_path->_v, NULL);
	} else {
		path.concat(s->_path._v, c->_wd._v, "/", cmd_path->_v, NULL);
	}

	i = path._i;
	while (i >= 0 && path._v[i] == '/') {
		i--;
	}
	while (i >= 0 && path._v[i] != '/') {
		i--;
	}
	if (i < 0) {
		return -1;
	}

	path._v[i]	= '\0';
	tmp_i		= path._i;
	path._i		= i;

	(*node) = s->_dir.get_node(&path, s->_path._v, s->_path._i);
	if (!(*node)) {
		return -1;
	}

	path._v[i]	= '/';
	path._i		= tmp_i;

	rpath->append(&s->_path);
	s->_dir.get_parent_path(rpath, (*node));

	i++;
	dirname->copy_raw(&path._v[i], path._i - i);
	rpath->append(dirname);

	if (LIBVOS_DEBUG) {
		printf("[LIBVOS::FTPD____] get_path_node.realpath: %s\n", rpath->_v);
		printf("                   get_path_node.dirname : %s\n", dirname->_v);
	}

	return 0;
}

/**
 * @method	: FTPD::on_accept
 * @param	:
 *	> svr	: pointer to FTPD object.
 *	> clt	: pointer to FTPClient object.
 * @desc	: called after server accepting new client connection.
 */
void FTPD::on_accept(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}
	c->_conn_stat = FTP_STT_CONNECTED;
	c->_wd.copy_raw("/");
	c->reply_raw(CODE_220, _FTP_reply_msg[CODE_220], NULL);
}

/**
 * @method	: FTPD::on_cmd_USER
 * @param	:
 *	> srv	: pointer to FTPD object.
 *	> clt	: pointer to FTPClient object.
 * @desc	: called when user send USER command.
 */
void FTPD::on_cmd_USER(FTPD* s, FTPClient* c)
{
	int x;

	if (!s || !c) {
		return;
	}
	if (c->_conn_stat != FTP_STT_CONNECTED) {
		return;
	}
	if (s->_auth_mode == AUTH_NOLOGIN) {
		c->_s		= CODE_230;
		c->_conn_stat	= FTP_STT_LOGGED_IN;
		goto out;
	}

	x = FTPUser::IS_NAME_EXIST(s->_users, &c->_cmnd._parm);
	if (!x) {
		c->_s = CODE_530;
	} else {
		c->_s = CODE_331;
	}
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_PASS(FTPD* s, FTPClient* c)
{
	int x;

	if (!s || !c) {
		return;
	}
	if (c->_cmnd_last._code != FTP_CMD_USER) {
		c->_s = CODE_503;
		goto out;
	}

	x = FTPUser::IS_EXIST(s->_users, &c->_cmnd_last._parm
				, &c->_cmnd._parm);
	if (!x) {
		c->_s = CODE_530;
	} else {
		c->_s		= CODE_230;
		c->_conn_stat	= FTP_STT_LOGGED_IN;
	}
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_SYST(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}
	c->reply_raw(CODE_215, _FTP_reply_msg[CODE_215], NULL);
}

void FTPD::on_cmd_TYPE(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}
	c->reply_raw(CODE_200, _FTP_reply_msg[CODE_200]
			, _FTP_add_reply_msg[TYPE_ALWAYS_BIN]);
}

void FTPD::on_cmd_MODE(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}
	c->reply_raw(CODE_200, _FTP_reply_msg[CODE_200]
			, _FTP_add_reply_msg[MODE_ALWAYS_STREAM]);
}

void FTPD::on_cmd_STRU(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}
	c->reply_raw(CODE_200, _FTP_reply_msg[CODE_200]
			, _FTP_add_reply_msg[STRU_ALWAYS_FILE]);
}

void FTPD::on_cmd_FEAT(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}
	c->reply_raw(CODE_211, _FTP_reply_msg[CODE_211], NULL);
}

void FTPD::on_cmd_SIZE(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	Buffer		size;
	Buffer		cwd;
	Buffer*		parm		= &c->_cmnd._parm;
	DirNode*	node		= NULL;

	if (parm->is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	if (parm->_v[0] == '/') {
		cwd.concat(s->_path._v, parm->_v, NULL);
	} else {
		cwd.concat(s->_path._v, c->_wd._v, "/", parm->_v, NULL);
	}

	node = s->_dir.get_node(&cwd, s->_path._v, s->_path._i);
	if (!node) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
	} else {
		c->_s		= CODE_213;
		size.appendi(node->_size);
		c->_rmsg_plus	= size._v;
	}
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_MDTM(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	Buffer		tm;
	Buffer		cwd;
	struct tm	gmt;
	Buffer*		parm		= &c->_cmnd._parm;
	DirNode*	node		= NULL;
	struct tm*	pgmt		= NULL;

	if (parm->is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	if (parm->_v[0] == '/') {
		cwd.concat(s->_path._v, parm->_v, NULL);
	} else {
		cwd.concat(s->_path._v, c->_wd._v, "/", parm->_v, NULL);
	}

	node = s->_dir.get_node(&cwd, s->_path._v, s->_path._i);
	if (!node) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
	} else {
		pgmt = gmtime_r(&node->_mtime, &gmt);
		if (!pgmt) {
			c->_s = CODE_451;
			goto out;
		}

		tm.aprint("%d%02d%02d%02d%02d%02d"
				, 1900 + gmt.tm_year, gmt.tm_mon, gmt.tm_mday
				, gmt.tm_hour, gmt.tm_min, gmt.tm_sec);

		c->_s		= CODE_213;
		c->_rmsg_plus	= tm._v;
	}
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_CWD(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	Buffer		cwd;
	Buffer*		parm	= &c->_cmnd._parm;
	DirNode*	node	= NULL;

	if (parm->is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	if (parm->_v[0] == '/') {
		cwd.concat(s->_path._v, parm->_v, NULL);
	} else {
		cwd.concat(s->_path._v, c->_wd._v, "/", parm->_v, NULL);
	}

	node = s->_dir.get_node(&cwd, s->_path._v, s->_path._i);
	if (!node) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
	} else {
		c->_s		= CODE_250;
		c->_wd_node	= node;

		c->_wd.reset();
		s->_dir.get_parent_path(&c->_wd, node);

		if (LIBVOS_DEBUG) {
			printf("[LIBVOS::FTPD____] CWD : %s\n", c->_wd._v);
		}
	}
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_CDUP(FTPD* s, FTPClient* c)
{
	c->_wd_node = c->_wd_node->_parent;
	c->_wd.reset();
	s->_dir.get_parent_path(&c->_wd, c->_wd_node);
	c->reply_raw(CODE_250, _FTP_reply_msg[CODE_250], NULL);
}

void FTPD::on_cmd_PWD(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}
	c->reply_raw(CODE_257, _FTP_reply_msg[CODE_257], c->_wd._v);
}

int FTPD::GET_PASV_PORT()
{
	return ((rand() % 64511) + 1025);
}

void FTPD::on_cmd_PASV(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		p1;
	int		p2;
	int		pasv_port	= GET_PASV_PORT();
	Buffer		pasv_addr;
	Socket*		pasv_sock	= NULL;

	if (!c->_sock) {
		if (LIBVOS_DEBUG) {
			printf("[LIBVOS::FTPD____] on_cmd_PASV: client socket null!\n");
		}
		return;
	}

	pasv_sock = new Socket();
	if (!pasv_sock) {
		goto err;
	}

	c->_s = pasv_sock->create_tcp();
	if (c->_s < 0) {
		goto err;
	}

	pasv_addr.copy(&c->_sock->_name);

	do {
		c->_s = pasv_sock->bind_listen(pasv_addr._v, pasv_port);
		if (c->_s < 0) {
			if (errno != EADDRINUSE) {
				goto err;
			}
			pasv_port = GET_PASV_PORT();
		}
	} while (c->_s != 0);

	c->_s = pasv_addr.subc('.', ',');
	if (c->_s != 3) {
		goto err;
	}

	p1 = pasv_port / 256;
	p2 = pasv_port % 256;

	pasv_addr.aprint(",%d,%d", p1, p2);

	c->_psrv = pasv_sock;

	FD_SET(pasv_sock->_d, &s->_fds_all);
	if (pasv_sock->_d >= s->_fds_max) {
		s->_fds_max = pasv_sock->_d + 1;
	}

	if (LIBVOS_DEBUG) {
		printf("[LIBVOS::FTPD____] PASV: %s\n", pasv_addr._v);
	}

	c->_s		= CODE_227;
	c->_rmsg_plus	= pasv_addr._v;
	c->_rmsg	= _FTP_reply_msg[c->_s];
	c->reply();

	return;
err:
	if (pasv_sock) {
		delete pasv_sock;
	}
	c->_s		= 451;
	c->_rmsg	= _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_LIST(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		x;
	Buffer		path;
	struct tm	gmt;
	Buffer*		cmd_path	= &c->_cmnd._parm;
	DirNode*	node		= NULL;
	Socket*		pasv_c		= NULL;
	struct tm*	pgmt		= NULL;

	if (!c->_psrv || !c->_pclt) {
		c->_s = CODE_425;
		goto out;
	}

	if (cmd_path->_v[0] == '/') {
		path.concat(s->_path._v, cmd_path->_v, NULL);
	} else {
		path.concat(s->_path._v, c->_wd._v, "/", cmd_path->_v, NULL);
	}

	x = s->_dir.refresh_by_path(&path);
	if (x < 0) {
		c->_s		= CODE_450;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
		goto out;
	}

	node = s->_dir.get_node(&path, s->_path._v, s->_path._i);
	if (!node) {
		c->_s		= CODE_450;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
		goto out;
	}

	pasv_c = c->_pclt;
	pasv_c->reset();

	c->reply_raw(CODE_150, _FTP_reply_msg[CODE_150], NULL);

	if (!node->is_dir()) {
		pgmt = gmtime_r(&node->_mtime, &gmt);
		if (pgmt) {
			pasv_c->aprint("%d\t%d\t%d\t%ld\t"
					, node->_mode
					, node->_uid, node->_gid
					, node->_size);

			pasv_c->aprint("%d-%02d-%02d %02d:%02d\t"
					, 1900 + gmt.tm_year
					, gmt.tm_mon, gmt.tm_mday
					, gmt.tm_hour, gmt.tm_min);
			pasv_c->append(&node->_name);
			pasv_c->append_raw("\r\n");
		}
	} else {
		if (node->_link) {
			node = node->_link->_child;
		} else {
			node = node->_child;
		}
		while (node) {
			pgmt = gmtime_r(&node->_mtime, &gmt);
			if (pgmt) {
				pasv_c->aprint("%d\t%d\t%d\t%ld\t"
					, node->_mode
					, node->_uid, node->_gid
					, node->_size);

				pasv_c->aprint("%d-%02d-%02d %02d:%02d\t"
						, 1900 + gmt.tm_year
						, gmt.tm_mon, gmt.tm_mday
						, gmt.tm_hour, gmt.tm_min);
				pasv_c->append(&node->_name);
				pasv_c->append_raw("\r\n");
			}
			node = node->_next;
		}
	}
	pasv_c->send_raw(0);

	if (c->_psrv) {
		delete c->_psrv;
		c->_psrv = NULL;
	}

	c->_s		= CODE_226;
out:
	c->_rmsg	= _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_NLST(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		x;
	Buffer		path;
	Buffer*		cmd_path	= &c->_cmnd._parm;
	DirNode*	node		= NULL;
	Socket*		pasv_c		= NULL;

	if (!c->_psrv || !c->_pclt) {
		c->_s = CODE_425;
		goto out;
	}

	if (cmd_path->_v[0] == '/') {
		path.concat(s->_path._v, cmd_path->_v, NULL);
	} else {
		path.concat(s->_path._v, c->_wd._v, "/", cmd_path->_v, NULL);
	}

	x = s->_dir.refresh_by_path(&path);
	if (x < 0) {
		c->_s		= CODE_450;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
		goto out;
	}

	node = s->_dir.get_node(&path, s->_path._v, s->_path._i);
	if (!node) {
		c->_s		= CODE_450;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
		goto out;
	}

	pasv_c = c->_pclt;
	pasv_c->reset();

	c->reply_raw(CODE_150, _FTP_reply_msg[CODE_150], NULL);

	if (!node->is_dir()) {
		pasv_c->append(&node->_name);
		pasv_c->append_raw("\r\n");
	} else {
		if (node->_link) {
			node = node->_link->_child;
		} else {
			node = node->_child;
		}
		while (node) {
			pasv_c->append(&node->_name);
			pasv_c->append_raw("\r\n");
			node = node->_next;
		}
	}
	pasv_c->send_raw(0);

	if (c->_psrv) {
		delete c->_psrv;
		c->_psrv = NULL;
	}

	c->_s = CODE_226;
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_RETR(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		n;
	Buffer		path;
	Buffer		node_name;
	File		file;
	Buffer*		cmd_parm	= &c->_cmnd._parm;
	DirNode*	node		= NULL;
	Socket*		pasv_c		= NULL;

	if (!c->_psrv || !c->_pclt) {
		c->_s = CODE_425;
		goto out;
	}

	if (cmd_parm->is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	if (cmd_parm->_v[0] == '/') {
		path.concat(s->_path._v, cmd_parm->_v, NULL);
	} else {
		path.concat(s->_path._v, c->_wd._v, "/", cmd_parm->_v, NULL);
	}

	node = s->_dir.get_node(&path, s->_path._v, s->_path._i);
	if (!node) {
		c->_s		= CODE_450;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
		goto out;
	}

	if (node->is_dir()) {
		on_cmd_LIST(s, c);
		return;
	}

	n = file.open_ro(path._v);
	if (n < 0) {
		c->_s = CODE_451;
		goto out;
	}

	pasv_c = c->_pclt;
	pasv_c->reset();

	c->reply_raw(CODE_150, _FTP_reply_msg[CODE_150], NULL);

	n = file.read();
	while (n > 0) {
		pasv_c->write(&file);
		n = file.read();
	}
	if (c->_psrv) {
		delete c->_psrv;
		c->_psrv = NULL;
	}

	c->_s = CODE_226;
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_STOR(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		x;
	Buffer		rpath;
	Buffer		node_name;
	File		file;
	DirNode*	list		= NULL;
	Socket*		pasv_c		= NULL;

	if (!c->_psrv || !c->_pclt) {
		c->_s = CODE_425;
		goto out;
	}

	if (c->_cmnd._parm.is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	x = get_path_node(s, c, &c->_cmnd._parm, &list, &rpath, &node_name);
	if (x < 0) {
		c->_s = CODE_501;
		goto out;
	}

	x = file.open_wo(rpath._v);
	if (x < 0) {
		c->_s = CODE_451;
		goto out;
	}

	pasv_c = c->_pclt;

	c->reply_raw(CODE_150, _FTP_reply_msg[CODE_150], NULL);

	x = pasv_c->read();
	while (x > 0) {
		file.write(pasv_c);
		x = pasv_c->read();
	}

	if (c->_psrv) {
		delete c->_psrv;
		c->_psrv = NULL;
	}

	x = DirNode::INSERT_CHILD(list, rpath._v, node_name._v);
	if (x < 0) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= strerror(errno);
		goto out;
	}

	c->_s = CODE_226;
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_DELE(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		x;
	Buffer		rpath;
	Buffer		dirname;
	DirNode*	list = NULL;

	if (c->_cmnd._parm.is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	x = get_path_node(s, c, &c->_cmnd._parm, &list, &rpath, &dirname);
	if (x != 0) {
		c->_s = CODE_501;
		goto out;
	}

	x = unlink(rpath._v);
	if (x < 0) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= strerror(errno);
		goto out;
	}

	DirNode::REMOVE_CHILD_BY_NAME(list, dirname._v);

	c->_s = CODE_250;
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_RNFR(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	Buffer		path;
	Buffer*		cmd_path = &c->_cmnd._parm;
	DirNode*	node = NULL;

	if (cmd_path->is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	if (cmd_path->_v[0] == '/') {
		path.concat(s->_path._v, cmd_path->_v, NULL);
	} else {
		path.concat(s->_path._v, c->_wd._v, "/", cmd_path->_v, NULL);
	}

	node = s->_dir.get_node(&path, s->_path._v, s->_path._i);
	if (!node) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
	} else {
		c->_s = CODE_350;
	}
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_RNTO(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		x;
	Buffer		path;
	Buffer		from;
	Buffer		to;
	Buffer		name_from;
	Buffer		name_to;
	Buffer*		last_parm	= &c->_cmnd_last._parm;
	DirNode*	node		= NULL;
	DirNode*	node_from	= NULL;
	DirNode*	node_to		= NULL;

	if (c->_cmnd._parm.is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	if (c->_cmnd_last._code != FTP_CMD_RNFR) {
		c->_s = CODE_503;
		goto out;
	}

	/* get from path */
	if (last_parm->_v[0] == '/') {
		path.concat(s->_path._v, last_parm->_v, NULL);
	} else {
		path.concat(s->_path._v, c->_wd._v, "/", last_parm->_v, NULL);
	}

	node = s->_dir.get_node(&path, s->_path._v, s->_path._i);
	if (!node) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= _FTP_add_reply_msg[NODE_NOT_FOUND];
		goto out;
	}

	x = get_path_node(s, c, last_parm, &node_from, &from, &name_from);
	if (x != 0) {
		c->_s = CODE_501;
		goto out;
	}

	/* get 'to' path */
	x = get_path_node(s, c, &c->_cmnd._parm, &node_to, &to, &name_to);
	if (x != 0) {
		c->_s = CODE_501;
		goto out;
	}

	if (name_to.is_empty()) {
		to.appendc('/');
		to.append(&name_from);
	}

	if (LIBVOS_DEBUG) {
		printf("[LIBVOS::FTPD____] RENAME from : %s\n", from._v);
		printf("                          to   : %s\n", to._v);
	}

	x = rename(from._v, to._v);
	if (x < 0) {
		c->_s		= CODE_553;
		c->_rmsg_plus	= strerror(errno);
	} else {
		c->_s = CODE_250;
		if (! name_to.is_empty()) {
			node->_name.copy(&name_to);
		} else {
			DirNode::REMOVE_CHILD_BY_NAME(node_from, name_from._v);
			DirNode::INSERT_CHILD(node_to, to._v, name_from._v);
		}
	}
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_RMD(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		x;
	Buffer		rpath;
	Buffer		dirname;
	DirNode*	list = NULL;

	if (c->_cmnd._parm.is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	x = get_path_node(s, c, &c->_cmnd._parm, &list, &rpath, &dirname);
	if (x != 0) {
		c->_s = CODE_550;
		goto out;
	}

	x = rmdir(rpath._v);
	if (x < 0) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= strerror(errno);
		goto out;
	}

	DirNode::REMOVE_CHILD_BY_NAME(list, dirname._v);

	c->_s = CODE_250;
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_MKD(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}

	int		x;
	Buffer		rpath;
	Buffer		dirname;
	DirNode*	list		= NULL;

	if (c->_cmnd._parm.is_empty()) {
		c->_s = CODE_501;
		goto out;
	}

	x = get_path_node(s, c, &c->_cmnd._parm, &list, &rpath, &dirname);
	if (x != 0) {
		c->_s = CODE_550;
		goto out;
	}

	x = Dir::CREATE(rpath._v);
	if (x < 0) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= strerror(errno);
		goto out;
	}

	x = DirNode::INSERT_CHILD(list, rpath._v, dirname._v);
	if (x < 0) {
		c->_s		= CODE_550;
		c->_rmsg_plus	= strerror(errno);
	} else {
		c->_s		= CODE_257;
		c->_rmsg_plus	= (const char*) rpath._v;
	}
out:
	c->_rmsg = _FTP_reply_msg[c->_s];
	c->reply();
}

void FTPD::on_cmd_QUIT(FTPD* s, FTPClient* c)
{
	if (!s || !c) {
		return;
	}
	c->reply_raw(CODE_221, _FTP_reply_msg[CODE_221], NULL);
	s->client_del(c);
}

void FTPD::on_cmd_unknown(FTPClient* c)
{
	c->reply_raw(CODE_502, _FTP_reply_msg[CODE_502], 0);
}

} /* namespace::vos */
