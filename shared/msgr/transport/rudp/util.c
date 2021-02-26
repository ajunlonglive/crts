#include "posix.h"

#include "shared/msgr/transport/rudp/util.h"

bool
seq_gt(uint16_t s1, uint16_t s2)
{
	/* Gaffer on Games: If their difference is less than 1/2 the maximum
	 * sequence number value, then they must be close together - so we just
	 * check if one is greater than the other, as usual. However, if they
	 * are far apart, their difference will be greater than 1/2 the max
	 * sequence, then we paradoxically consider the sequence number more
	 * recent if it is less than the current sequence number.  */
	return ((s1 > s2) && (s1 - s2 <= UINT16_MAX / 2))
	       || ((s1 < s2) && (s2 - s1 > UINT16_MAX / 2));
}

bool
seq_lt(uint16_t s1, uint16_t s2)
{
	return s1 != s2 && !seq_gt(s1, s2);
}

uint16_t
seq_diff(uint16_t a, uint16_t b)
{
	if (b > a) {
		return (UINT16_MAX - b) + a;
	} else {
		return a - b;
	}
}
