#include "../SSVReader.hpp"

int main (int argc, char** argv)
{
	register int s;
	vos::SSVReader reader;

	s = reader.load ("./hosts");
	if (s != 0) {
		return -1;
	}

	reader._rows->dump ();

	reader.reset ();
	reader._comment_c = '#';

	s = reader.load ("./hosts");

	reader._rows->dump ();

	return s;
}
