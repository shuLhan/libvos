#include "libvos.hpp"

namespace vos {

int LIBVOS_DEBUG = getenv("LIBVOS_DEBUG") == NULL
			? 0
			: atoi(getenv("LIBVOS_DEBUG"));

const char *_errmsg[N_ERRCODE] = {
	"\0",
	"\0",

	"[LIBVOS] Error: Invalid program parameter!\n",
	"[LIBVOS] Error: Out of memory!\n",
	"[LIBVOS] Error: Invalid 'printf' arguments!\n",
	"[LIBVOS] Error: Unknown host or address!\n",

	"[LIBVOS] File error: cannot open file!\n",
	"[LIBVOS] File error: at reading on file!\n",
	"[LIBVOS] File error: at writing on file!\n",
	"[LIBVOS] File error: at seeking on file!\n",
	"[LIBVOS] File: end of file reached.\n",

	"[LIBVOS] Error at loading configuration file!\n",

	"[LIBVOS] Socket error: cannot create socket!\n",
	"[LIBVOS] Socket error: invalid address!\n",
	"[LIBVOS] Socket error: cannot resolve address!\n",
	"[LIBVOS] Socket error: cannot reuse address!\n",

	"[LIBVOS] Socket error: bind error!\n",
	"[LIBVOS] Socket error: cannot listen to socket descriptor!\n",
	"[LIBVOS] Socket error: cannot connect to host!\n",
	"[LIBVOS] Socket error: read operation failed!\n",

	"[LIBVOS] Socket error: write operation failed!\n",
	"[LIBVOS] Socket error: select operation failed!\n",
	"[LIBVOS] Socket error: failed to accept new connection!\n",
	"[LIBVOS] Socket error: timeout reached!\n",

	"[LIBVOS] Record error: invalid column!\n",
	"[LIBVOS] Record rejected!\n",

	"[LIBVOS] OCI Error: %d - %s\n"
};

const char *_stat_msg[N_STAT] = {
	"[OK]",
	"[FAIL]"
};

} /* namespace::vos */
