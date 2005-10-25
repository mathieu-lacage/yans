/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "utils.h"

uint16_t 
utils_hton_16 (uint16_t v)
{
	uint8_t array[2];
	array[0] = v & 0xff;
	array[1] = (v >> 8) & 0xff;
	return *((uint16_t *)array);
}
uint32_t 
utils_hton_32 (uint32_t v)
{
	uint8_t array[4];
	array[0] = v & 0xff;
	array[1] = (v >> 8) & 0xff;
	array[2] = (v >> 16) & 0xff;
	array[3] = (v >> 24) & 0xff;
	return *((uint32_t *)array);
}
uint16_t 
utils_ntoh_16 (uint16_t v)
{
	uint16_t val;
	uint8_t *array;
	array = (uint8_t *)&v;
	val = array[0] | (array[1] << 8);
	return val;
}
uint32_t 
utils_ntoh_32 (uint32_t v)
{
	uint32_t val = 0;
	uint8_t *array = (uint8_t *)&v;
	val |= array[0] << 0;
	val |= array[1] << 8;
	val |= array[2] << 16;
	val |= array[3] << 24;
	return val;
}

#define ASCII_DOT (0x2e)
#define ASCII_ZERO (0x30)
#define ASCII_A (0x41)
#define ASCII_Z (0x5a)
#define ASCII_a (0x61)
#define ASCII_z (0x7a)
#define ASCII_MINUS (0x2d)


uint32_t 
ascii_to_ipv4_host (char const *address)
{
	uint32_t host = 0;
	while (true) {
		uint8_t byte = 0;
		while (*address != ASCII_DOT &&
		       *address != 0) {
			byte *= 10;
			byte += *address - ASCII_ZERO;
			address++;
		}
		host <<= 8;
		host |= byte;
		if (*address == 0) {
			break;
		}
		address++;
	}
	return host;
}


char
ascii_to_low_case (char c)
{
	if (c >= ASCII_a && c <= ASCII_z) {
		return c;
	} else if (c >= ASCII_A && c <= ASCII_Z) {
		return c + (ASCII_a - ASCII_A);
	} else {
		return c;
	}
}

void 
ascii_to_mac_network (char const *str, uint8_t address[6])
{
	uint8_t i = 0;
	while (*str != 0 && i < 6) {
		uint8_t byte = 0;
		while (*str != ASCII_MINUS ) {
			byte <<= 8;
			char low = ascii_to_low_case (*str);
			byte |= low - ASCII_a;
			str++;
		}
		address[6-i] = byte;
		str++;
		i++;
	}
}

uint16_t 
calculate_checksum (uint8_t *buffer, uint16_t size)
{
	/* see RFC 1071 to understand this code. */
	uint32_t sum = 0;
	uint32_t *data = (uint32_t *) buffer;
	for (uint16_t i = 0; i < size; i += 4) {
		sum += data[i];
	}
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}
	uint16_t checksum =  ~sum;
	return checksum;

}


#ifdef RUN_SELF_TESTS
#include "test.h"
UtilsTest::UtilsTest (TestManager *manager)
	: Test (manager)
{}
UtilsTest::~UtilsTest ()
{}

bool
UtilsTest::test_ipv4_ascii_to_host (char const *str, uint32_t expected)
{
	if (ascii_to_ipv4_host (str) != expected) {
		failure () << "Utils ascii to host --"
			   << " for: \"" << str << "\""
			   << " expected: " << expected
			   << " got: " << ascii_to_ipv4_host (str)
			   << std::endl;
		return false;
	}
	return true;
}

#define TEST_IPV4_ASCII_TO_HOST(a,b)  \
if (!test_ipv4_ascii_to_host (a,b)) { \
	ok = false;                   \
}

bool 
UtilsTest::run_tests (void)
{
	bool ok = true;
	TEST_IPV4_ASCII_TO_HOST ("255.255.255.255", 0xffffffff);
	TEST_IPV4_ASCII_TO_HOST ("255.255.255.0", 0xffffff00);
	TEST_IPV4_ASCII_TO_HOST ("255.255.255.00", 0xffffff00);
	TEST_IPV4_ASCII_TO_HOST ("255.255.255.000", 0xffffff00);
	TEST_IPV4_ASCII_TO_HOST ("255.255.255.0000", 0xffffff00);
	TEST_IPV4_ASCII_TO_HOST ("255.255.0.255", 0xffff00ff);
	TEST_IPV4_ASCII_TO_HOST ("192.168.0.1", 0xc0a80001);
	TEST_IPV4_ASCII_TO_HOST ("0.168.0.1", 0x00a80001);
	return ok;
}

#endif /* RUN_SELF_TESTS */
