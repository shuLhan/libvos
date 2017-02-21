#include "test.hh"
#include "../SockAddr.hh"

#define TEST_0_ADDR4		"0.0.0.0"
#define TEST_0_ADDR6		"::"
#define TEST_0_PORT		0
#define	TEST_0_CHARS_EXP	TEST_0_ADDR4 ":0 " TEST_0_ADDR6 ":0"

#define TEST_1_ADDR4		"8.8.8.8"
#define TEST_1_ADDR6		"2001:0db8:0000:0042:0000:8a2e:0370:7334"
#define TEST_1_ADDR6_EXP	"2001:db8:0:42:0:8a2e:370:7334"
#define TEST_1_PORT		53
#define TEST_1_CHARS_EXP_1	TEST_1_ADDR4 ":53 " TEST_0_ADDR6 ":0"
#define TEST_1_CHARS_EXP_2	TEST_1_ADDR4 ":53 " TEST_1_ADDR6_EXP ":53"

#define TEST_2_PORT		70000
#define TEST_2_PORT_EXP		4464

using vos::SockAddr;

SockAddr sa;

void check_ip4(int exp_type, const char* exp_addr, uint16_t exp_port)
{
	assert((sa._t & exp_type) == exp_type);
	assert(strcmp(exp_addr, sa.get_address(AF_INET)) == 0);
	assert(exp_port == sa.get_port(AF_INET));
}

void check_ip6(int exp_type, const char* exp_addr, uint16_t exp_port)
{
	assert((sa._t & exp_type) == exp_type);
	assert(strcmp(exp_addr, sa.get_address(AF_INET6)) == 0);
	assert(sa.get_port(AF_INET6) == exp_port);
}

void test_IS_IPV4()
{
	assert(SockAddr::IS_IPV4("0.0.0.0") == 1);
	assert(SockAddr::IS_IPV4("127.0.0.1") == 1);
	assert(SockAddr::IS_IPV4("192.168.1.0") == 1);
	assert(SockAddr::IS_IPV4("192.168.1.1") == 1);
	assert(SockAddr::IS_IPV4("192.168.1.255") == 1);
	assert(SockAddr::IS_IPV4("192.168.1.265") == 0);
	assert(SockAddr::IS_IPV4("192.168.1a.254") == 0);
	assert(SockAddr::IS_IPV4("192.1680.1.254") == 0);
	assert(SockAddr::IS_IPV4("1.168.1.256") == 0);
	assert(SockAddr::IS_IPV4("1.168.1.2520") == 0);
	assert(SockAddr::IS_IPV4("1.168.1.") == 0);
	assert(SockAddr::IS_IPV4("192.168.1") == 0);
	assert(SockAddr::IS_IPV4("192..168.1") == 0);
}

int main()
{
	check_ip4(0, TEST_0_ADDR4, TEST_0_PORT);
	check_ip6(0, TEST_0_ADDR6, TEST_0_PORT);

	assert(strcmp(TEST_0_CHARS_EXP, sa.chars()) == 0);

	//
	// Set ipv4
	//
	sa.set(AF_INET, TEST_1_ADDR4, TEST_1_PORT);

	check_ip4(AF_INET, TEST_1_ADDR4, TEST_1_PORT);
	check_ip6(AF_INET, TEST_0_ADDR6, TEST_0_PORT);

	assert(strcmp(TEST_1_CHARS_EXP_1, sa.chars()) == 0);

	//
	// Set ipv6
	//
	sa.set(AF_INET6, TEST_1_ADDR6, TEST_1_PORT);

	check_ip4(AF_INET, TEST_1_ADDR4, TEST_1_PORT);
	check_ip6(AF_INET6, TEST_1_ADDR6_EXP, TEST_1_PORT);

	// test set port out of range
	sa.set_port(AF_INET, (uint16_t) TEST_2_PORT);

	// the port in addr4 will be overflow, since its uin16_t
	check_ip4(AF_INET, TEST_1_ADDR4, TEST_2_PORT_EXP);

	// the port in addr6 should not change
	check_ip6(AF_INET6, TEST_1_ADDR6_EXP, TEST_1_PORT);

	test_IS_IPV4();

	return 0;
}
