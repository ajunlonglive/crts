#ifndef SHARED_UTIL_UTIL_H
#define SHARED_UTIL_UTIL_H

#include <stdint.h>

int32_t clamp(int32_t v, int32_t min, int32_t max);
float fclamp(float v, float min, float max);
float maxf(float a, float b);
#endif
