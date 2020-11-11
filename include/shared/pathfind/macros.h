#ifndef SHARED_PATHFIND_MACROS_H
#define SHARED_PATHFIND_MACROS_H

/* helpers for efficent getting/setting of a sub-byte array of "arbitrary"
 * granularity.  usage of shifts and masks rather than division and modulo
 * might be overkill :) */

#define SUB_BYTE_GET(es, div, mod, mul, mask, i) ((es[(i) >> (div)] >> (((i) & mod) * mul)) & mask)
#define SUB_BYTE_SET(es, div, mod, mul, i, v) (es[(i) >> (div)] |= (v << (((i) & mod) * mul)))

/* indexes 1-bit chunks */
#define SB1_GET(es, i) SUB_BYTE_GET(es, 3, 7, 1, 1, i)
#define SB1_SET(es, i, v) SUB_BYTE_SET(es, 3, 7, 1, i, v)

/* indexes 4-bit chunks */
#define SB4_GET(es, i) SUB_BYTE_GET(es, 1, 1, 4, 0xf, i)
#define SB4_SET(es, i, v) SUB_BYTE_SET(es, 1, 1, 4, i, v)

/* getters for adjacent x-col indices */
#define LEFT_OF(i) ((i) - 16)
#define RIGHT_OF(i) ((i) + 16)
#define ABOVE(i) ((i) - 1)
#define BELOW(i) ((i) + 1)

#define TRAV_LEFT_OF(trav, i) (i > 15 ? (SB1_GET(trav, LEFT_OF(i))) : 0)
#define TRAV_RIGHT_OF(trav, i) (i < 240 ? (SB1_GET(trav, RIGHT_OF(i))) : 0)
#define TRAV_ABOVE(trav, i) (i & 15 ? (SB1_GET(trav, ABOVE(i))) : 0)
#define TRAV_BELOW(trav, i) ((i + 1) & 15 ? (SB1_GET(trav, BELOW(i))) : 0)

#define HAS_LEFT_OF(i) (i > 15)
#define HAS_RIGHT_OF(i) (i < 240)
#define HAS_ABOVE(i) (i & 15)
#define HAS_BELOW(i) ((i + 1) & 15)

#endif
