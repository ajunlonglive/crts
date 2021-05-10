#include "posix.h"

#include "shared/platform/posix/path.h"

bool
platform_path_is_relative(const char *path)
{
	return *path != '/';
}
