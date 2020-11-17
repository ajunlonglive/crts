#ifndef SHARED_UTIL_ASSETS_H
#define SHARED_UTIL_ASSETS_H

#include <stddef.h>
#include <stdint.h>

struct file_data { const char *path; const uint8_t *data; size_t len; };

void asset_path_init(char *asset_path);
struct file_data* asset(const char *path);
const char *rel_to_abs_path(const char *relpath);
#endif
