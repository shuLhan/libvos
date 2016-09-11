#include "../Dlogger.hh"

using vos::Dlogger;

int main (int argc, char** argv)
{
	int i = 0;
	Dlogger dlog;

	dlog.open ("log", 2048);

	for (; i <= 300; i++) {
		dlog.it ("%d\n", i);
	}

	return 0;
}
