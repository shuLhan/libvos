#include "../Buffer.hpp"

using vos::Buffer;

int main ()
{
	Buffer a;

	a.copy_raw ("\n");

	a.dump_hex ();

	return 0;
}
