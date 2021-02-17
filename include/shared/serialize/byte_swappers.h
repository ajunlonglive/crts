#ifndef SHARED_SERIALIZE_BYTE_SWAPPERS_H
#define SHARED_SERIALIZE_BYTE_SWAPPERS_H

#include <stdint.h>

uint16_t host_to_net_16(uint16_t n);
uint16_t net_to_host_16(uint16_t n);
uint32_t host_to_net_32(uint32_t n);
uint32_t net_to_host_32(uint32_t n);
#endif
