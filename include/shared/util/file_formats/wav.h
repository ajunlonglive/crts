#ifndef SHARED_UTIL_FILE_FORMATS_WAV_H
#define SHARED_UTIL_FILE_FORMATS_WAV_H

#include <stdbool.h>
#include <stdint.h>

struct wav {
	double *data;
	uint32_t len;
};

bool load_wav(const char *path, struct wav *wav);
#endif
