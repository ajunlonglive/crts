#ifndef SHARED_SERIALIZE_LIMITS_H
#define SHARED_SERIALIZE_LIMITS_H

/* terrain height limits */
#define MAX_HEIGHT 256
#define MIN_HEIGHT -128
/* terrain height resolution */
#define STEPS ((MAX_HEIGHT - MIN_HEIGHT) * 1000)
#define MAX_COORD  4096
#define MIN_COORD -4096

#endif
