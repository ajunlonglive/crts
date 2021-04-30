#include "posix.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shared/types/hash.h"
#include "shared/util/assets.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

static struct file_data *embedded_files;
static size_t embedded_files_len = 0;
static const char **asset_manifest;
static size_t asset_manifest_len = 0;

#ifndef CRTS_ASSET_PATH
#define CRTS_ASSET_PATH ""
#endif

#define CHUNK_SIZE BUFSIZ

uint8_t *buffer = NULL;
size_t buffer_size = 0;

#define ASSET_PATHS_LEN 16
#define PATH_MAX 256
static struct {
	const char *path;
	size_t len;
} asset_paths[ASSET_PATHS_LEN] = { 0 };

static bool initialized;

void
assets_init(struct file_data *_embedded_files, size_t _embedded_files_len,
	const char *_asset_manifest[], const size_t _asset_manifest_len)
{
	embedded_files = _embedded_files;
	embedded_files_len = _embedded_files_len;
	asset_manifest = _asset_manifest;
	asset_manifest_len = _asset_manifest_len;
}

void
assets_list(void)
{
	const char *path;
	bool found;
	uint32_t i, j;

	uint8_t f[32] = { 0 };

	for (j = 0; j < asset_manifest_len; ++j) {
		found = false;

		for (i = 0; i < embedded_files_len; ++i) {
			path = embedded_files[i].path;
			if (strcmp(path, asset_manifest[j]) == 0) {
				found = true;
				break;
			}
		}

		if (found) {
			f[i / 8] |= (1 << (i & 7));
			printf("*%s\n", asset_manifest[j]);
		} else {
			printf(" %s\n", asset_manifest[j]);
		}
	}

	for (i = 0; i < embedded_files_len; ++i) {
		if (!(f[i / 8] & (1 << (i & 7)))) {
			printf("? %s\n", embedded_files[i].path);
		}
	}
}

const char *
rel_to_abs_path(const char *relpath)
{
	if (*relpath == '/') {
		return relpath;
	}

	static char cwd[PATH_MAX + 1] = { 0 },
		    buf[PATH_MAX * 2] = { 0 };
	if (!*cwd) {
		getcwd(cwd, PATH_MAX);
	}

	snprintf(buf, (PATH_MAX * 2) - 1, "%s/%s", cwd, relpath);

	return buf;
}

void
asset_path_init(char *asset_path)
{
	size_t i = 0;
	char *sep;

	assert(asset_path);

	while ((sep = strchr(asset_path, ':'))) {
		*sep = '\0';
		asset_paths[i].path = asset_path;
		asset_paths[i].len = strlen(asset_path);
		asset_path = sep + 1;
		++i;
	}

	asset_paths[i].path = asset_path;
	asset_paths[i].len = strlen(asset_path);
	initialized = true;
}

static struct file_data *
lookup_embedded_asset(const char *path)
{
	static struct file_data fd;

	size_t i;
	for (i = 0; i < embedded_files_len; ++i) {
		if (strcmp(path, embedded_files[i].path) == 0) {
			if (buffer_size < embedded_files[i].len) {
				buffer_size = embedded_files[i].len + 1;
				buffer = z_realloc(buffer, buffer_size);
			}

			memset(buffer, 0, buffer_size);
			memcpy(buffer, embedded_files[i].data, embedded_files[i].len);

			fd.path = path;
			fd.len = embedded_files[i].len;
			fd.data = buffer;

			return &fd;
		}
	}

	return NULL;
}

static struct file_data *
read_raw_asset(FILE *f, const char *path)
{

	static char ret_path[256] = { 0 };
	static struct file_data fd = { 0 };

	memcpy(ret_path, path, 255);
	fd.path = ret_path;

	memset(buffer, 0, buffer_size);

	fd.len = 0;

	size_t b = 1;
	while (b > 0) {
		if (buffer_size - fd.len < CHUNK_SIZE) {
			buffer_size += CHUNK_SIZE;

			buffer = z_realloc(buffer, buffer_size);
			memset(&buffer[fd.len], 0, buffer_size - fd.len);
		}

		assert(buffer_size - fd.len >= CHUNK_SIZE);
		b = fread(&buffer[fd.len], 1, CHUNK_SIZE, f);
		fd.len += b;
	}

	fd.data = buffer;

	fclose(f);

	return &fd;
}

struct file_data *
asset(const char *path)
{
	char pathbuf[PATH_MAX + 1];
	struct file_data *fdat;
	FILE *f;
	size_t i;

	if (!initialized) {
		char *ap;
		if (!(ap = getenv("CRTS_ASSET_PATH"))) {
			ap = CRTS_ASSET_PATH;
		}

		L("initializing asset path: '%s'", ap);
		asset_path_init(ap);
	}

	if (*path == '/') {
		goto read_file_from_absolute_path;
	}

#ifndef NDEBUG
	bool found_asset_in_manifest = false;

	for (i = 0; i < asset_manifest_len; ++i) {
		if (strcmp(path, asset_manifest[i]) == 0) {
			found_asset_in_manifest = true;
			break;
		}
	}

	if (!found_asset_in_manifest) {
		LOG_W("asset '%s' is not in manifest", path);
	}
#endif

	if ((fdat = lookup_embedded_asset(path))) {
		return fdat;
	}

	if (*path == '/') {
read_file_from_absolute_path:
		if (access(path, R_OK) == 0 && (f = fopen(path, "r"))) {
			return read_raw_asset(f, path);
		} else {
			return NULL;
		}
	}

	for (i = 0; i < ASSET_PATHS_LEN; ++i) {
		if (asset_paths[i].path == NULL) {
			continue;
		}

		snprintf(pathbuf, PATH_MAX, "%s/%s", asset_paths[i].path, path);

		if (access(pathbuf, R_OK) == 0 && (f = fopen(pathbuf, "r"))) {
			return read_raw_asset(f, pathbuf);
		}
	}

	LOG_W("failed to load asset '%s'", path);

	return NULL;
}
