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
