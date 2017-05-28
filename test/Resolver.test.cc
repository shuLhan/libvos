#include "test.hh"
#include "../Resolver.hh"

using vos::_RR_TYPE;

vos::Resolver res;

#define	DEF_IPV6	":::0"

#define TEST_0		"8.8.8.8"
#define TEST_0_CHARS	V_STR( TEST_0 ":53" )
#define TEST_0_EXP	CB( K(servers) SEP_KV SB(TEST_0_CHARS) )

#define TEST_1		"127.0.0.1:54"
#define TEST_1_CHARS	V_STR( TEST_1 )
#define TEST_1_EXP	CB( \
				K(servers) SEP_KV SB( \
					TEST_0_CHARS \
					SEP_ITEM \
					TEST_1_CHARS \
				) \
			)

#define TEST_2		TEST_0 SEP_ITEM TEST_1 SEP_ITEM "8.8.4.4:"
#define	TEST_2_CHARS	TEST_0_CHARS \
			SEP_ITEM \
			TEST_1_CHARS \
			SEP_ITEM \
			V_STR("8.8.4.4:53")
#define TEST_2_EXP	CB( \
				K(servers) SEP_KV SB( TEST_2_CHARS ) \
			)

void test_server_list()
{
	res.set_server(TEST_0);

	expectString(TEST_0_EXP, res.chars(), 0);

	res.add_server(TEST_1);

	expectString(TEST_1_EXP, res.chars(), 0);

	res.set_server(TEST_2);

	expectString(TEST_2_EXP, res.chars(), 0);
}

int main()
{
	test_server_list();
	return 0;
}
