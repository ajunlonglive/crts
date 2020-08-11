#include "posix.h"

#include <locale.h>

#include "terragen/gen/gen.h"
#include "terragen/opts.h"
#include "shared/util/log.h"
#ifdef OPENGL_UI
#include "terragen/opengl/ui.h"
#endif

int32_t
main(int32_t argc, char * const *argv)
{
	logfile = stderr;

	setlocale(LC_ALL, "");
	struct cmdline_opts opts = { 0 };
	parse_cmdline_opts(argc, argv, &opts);

	if (opts.interactive) {
#ifdef OPENGL_UI
		genworld_interactive(&opts.opts);
		return 0;
#else
		LOG_W("built without opengl, interactive mode unsupported");
		return 1;
#endif
	} else {
		struct chunks chunks, *_chunks = &chunks;
		chunks_init(&_chunks);

		struct terragen_ctx ctx = { 0 };

		terragen_init(&ctx, &opts.opts);

		terragen(&ctx, &chunks);
	}
}
