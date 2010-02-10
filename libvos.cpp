#include "libvos.hpp"

namespace vos {

int LIBVOS_DEBUG = getenv("LIBVOS_DEBUG") == NULL
			? 0
			: atoi(getenv("LIBVOS_DEBUG"));

} /* namespace::vos */
