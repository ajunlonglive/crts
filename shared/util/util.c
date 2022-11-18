#include "posix.h"

#include "shared/util/util.h"

int32_t
clamp(int32_t v, int32_t min, int32_t max)
{
	if (v > max) {
		return max;
	} else if (v < min) {
		return min;
	} else {
		return v;
	}
}

float
fclamp(float v, float min, float max)
{
	if (v > max) {
		return max;
	} else if (v < min) {
		return min;
	} else {
		return v;
	}
}

float
maxf(float a, float b)
{
	return a > b ? a : b;
}
