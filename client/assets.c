#include "posix.h"

#include "client/assets.h"
#include "shared/util/assets.h"

#ifdef INCLUDE_EMBEDDED_DATA
#include "embedded_data.h"
#else
static struct file_data embedded_files[] = { 0 };
static size_t embedded_files_len = 0;
#endif

#ifdef INCLUDE_EXPORTED_MANIFEST
#include "asset_manifest.h"
#else
static const char *asset_manifest[] = { 0 };
static const size_t asset_manifest_len = 0;
#endif

void
client_assets_init(void)
{
	assets_init(embedded_files, embedded_files_len,
		asset_manifest, asset_manifest_len);
}
