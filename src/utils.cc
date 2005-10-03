/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "utils.h"

uint16_t 
utils_htons (uint16_t v)
{
	uint8_t array[2];
	array[0] = v & 0xff;
	array[1] = (v >> 8) & 0xff;
	return *((uint16_t *)array);
}
uint32_t 
utils_htons (uint32_t v)
{
	uint8_t array[4];
	array[0] = v & 0xff;
	array[1] = (v >> 8) & 0xff;
	array[1] = (v >> 16) & 0xff;
	array[1] = (v >> 24) & 0xff;
	return *((uint32_t *)array);
}
uint16_t 
utils_nstoh (uint16_t v)
{
	uint16_t val;
	uint8_t *array;
	array = (uint8_t *)&v;
	val = array[0] | (array[1] << 8);
	return val;
}
uint32_t 
utils_nstoh (uint32_t v)
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

uint32_t 
ascii_to_ipv4_host (char const *address)
{
	uint32_t host = 0;
	while (*address != 0) {
		uint8_t byte = 0;
		while (*address != ASCII_DOT ) {
			byte *= 10;
			byte |= *address - ASCII_ZERO;
			address++;
		}
		host <<= 8;
		host |= byte;
		address++;
	}
	return host;
}

#define ASCII_A (0x41)
#define ASCII_Z (0x5a)
#define ASCII_a (0x61)
#define ASCII_z (0x7a)

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

#define ASCII_MINUS (0x2d)

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
