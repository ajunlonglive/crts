#include "posix.h"

#include <locale.h>

#include "genworld/gen.h"
#include "genworld/opts.h"
#include "shared/util/log.h"
#ifdef OPENGL_UI
#include "genworld/gl.h"
#endif

int32_t
main(int32_t argc, char * const *argv)
{
	setlocale(LC_ALL, "");
	struct genworld_opts opts = { 0 };
	parse_cmdline_opts(argc, argv, &opts);

	if (opts.interactive) {
#ifdef OPENGL_UI
		genworld_interactive(&opts.opts);
		return 0;
#else
		LOG_W("built without opengl, interactive mode unsupported");
		return 1;
#endif
	}

	return 0;
}
