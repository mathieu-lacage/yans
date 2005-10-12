/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

uint16_t utils_hton_16 (uint16_t v);
uint32_t utils_hton_32 (uint32_t v);
uint16_t utils_ntoh_16 (uint16_t v);
uint32_t utils_ntoh_32 (uint32_t v);

uint32_t ascii_to_ipv4_host (char const *address);
void ascii_to_mac_network (char const *str, uint8_t address[6]);

uint16_t calculate_checksum (uint8_t *buffer, uint16_t size);

#endif /* UTILS_H */
