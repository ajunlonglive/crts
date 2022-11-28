#include "posix.h"

#include <stdio.h>
#include <stdlib.h>

#include "shared/platform/common/dirs.h"

#define PATH_MAX 2048

const char *
platform_config_dir(void)
{
	static char buf[PATH_MAX], dir[PATH_MAX];
	const char *base;
	if (!(base = getenv("XDG_CONFIG_HOME"))) {
		const char *home;
		if (!(home = getenv("HOME"))) {
			return "./crts-config";
		}

		snprintf(buf, PATH_MAX, "%s/.config", home);
		base = buf;
	}

	snprintf(dir, PATH_MAX, "%s/crts", base);
	return dir;
}
