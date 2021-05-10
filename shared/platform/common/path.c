#include "posix.h"

#include "shared/platform/common/path.h"

#if defined(CRTS_PLATFORM_posix)
#include "shared/platform/posix/path.h"
#elif defined(CRTS_PLATFORM_windows)
#include "shared/platform/windows/path.h"
#else
#error "no valid platform defined"
#endif

bool
path_is_relative(const char *path)
{
	return platform_path_is_relative(path);
}
