#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <assert.h>

#include "shared/platform/windows/path.h"

bool
platform_path_is_relative(const char *path)
{
	assert(path && *path);

	return !(path[0] == '\\'
		 || ((path[1] == ':' && path[2] == '\\') && (
			     (path[0] >= 'A' && path[0] <= 'Z')
			     || (path[0] >= 'a' && path[0] <= 'z'))));
}
