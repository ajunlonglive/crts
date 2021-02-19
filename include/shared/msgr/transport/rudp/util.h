#ifndef SHARED_MSGR_TRANSPORT_RUDP_UTIL_H
#define SHARED_MSGR_TRANSPORT_RUDP_UTIL_H

#include <stdbool.h>
#include <stdint.h>

bool seq_gt(uint16_t a, uint16_t b);
bool seq_lt(uint16_t s1, uint16_t s2);
uint16_t seq_diff(uint16_t a, uint16_t b);
#endif
