/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "utils.h"

uint16_t 
htons (uint16_t v)
{
	uint8_t array[2];
	array[0] = v & 0xff;
	array[1] = (v >> 8) & 0xff;
	return *((uint16_t *)array);
}
uint32_t 
htons (uint32_t v)
{
	uint8_t array[4];
	array[0] = v & 0xff;
	array[1] = (v >> 8) & 0xff;
	array[1] = (v >> 16) & 0xff;
	array[1] = (v >> 24) & 0xff;
	return *((uint32_t *)array);
}
uint16_t 
nstoh (uint16_t v)
{
	uint16_t val;
	uint8_t *array;
	array = (uint8_t *)&v;
	val = array[0] | (array[1] << 8);
	return val;
}
uint32_t 
nstoh (uint32_t v)
{
	uint32_t val = 0;
	uint8_t *array = (uint8_t *)&v;
	val |= array[0] << 0;
	val |= array[1] << 8;
	val |= array[2] << 16;
	val |= array[3] << 24;
	return val;
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
