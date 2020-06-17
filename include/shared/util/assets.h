#ifndef CLIENT_ASSETS_H
#define CLIENT_ASSETS_H

#include <stddef.h>
#include <stdint.h>

struct file_data { const char *path; uint8_t *data; size_t len; };

void asset_path_init(char *asset_path);
struct file_data* asset(const char *path);
#endif
