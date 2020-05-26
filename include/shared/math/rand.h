#ifndef SHARED_MATH_RAND_H
#define SHARED_MATH_RAND_H
#include <stdint.h>
#include <stdbool.h>

uint32_t rand_uniform(uint32_t max);
bool rand_chance(uint32_t x);
void rand_set_seed(uint32_t seed);
#endif
