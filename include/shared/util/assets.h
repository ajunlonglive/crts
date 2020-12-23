#ifndef SHARED_UTIL_ASSETS_H
#define SHARED_UTIL_ASSETS_H

#include <stddef.h>
#include <stdint.h>

struct file_data { const char *path; const uint8_t *data; size_t len; };

void assets_init(struct file_data *embedded_files, size_t embedded_files_len,
	const char *asset_manifest[], const size_t asset_manifest_len);
void asset_path_init(char *asset_path);
struct file_data* asset(const char *path);
const char *rel_to_abs_path(const char *relpath);
#endif
