#include "posix.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "shared/util/inih.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

char *test_ini_file =
	"; comment\n"         //  1
	"\n"                  //  2
	"[sect_1]\n"          //  3
	"a = 1\n"             //  4
	"b   =    2 \n"       //  5
	"\n"                  //  6
	"; another comment\n" //  7
	"[sect_2]\n"          //  8
	"a = 3\n"             //  9
;

#define check_line(s1, k1, v1) assert(strcmp(s1, sect) == 0 && strcmp(k1, k) == 0 && strcmp(v1, v) == 0)

static bool
ini_parse_cb(void *_ctx, char *err, const char *sect, const char *k, const char *v, uint32_t line)
{
	size_t *found = _ctx;

	switch (line) {
	case 4:
		check_line("sect_1", "a", "1");
		break;
	case 5:
		check_line("sect_1", "b", "2 ");
		break;
	case 9:
		check_line("sect_2", "a", "3");
		break;
	default:
		assert(false);
		break;
	}

	++(*found);

	return true;
}

int
main(int argc, char *const *argv)
{
	size_t found = 0;
	struct file_data test_ini = { .path = "" };

	test_ini.len = strlen(test_ini_file);
	test_ini.data = z_malloc(test_ini.len);
	memcpy((void *)test_ini.data, test_ini_file, test_ini.len);

	log_set_lvl(log_debug);

	bool res = ini_parse(&test_ini, ini_parse_cb, &found);
	assert(found == 3);

	z_free((void *)test_ini.data);

	return !res; /* false = 0 */
}
