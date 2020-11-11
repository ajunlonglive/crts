#include "posix.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shared/pathfind/abstract.h"
#include "shared/pathfind/api.h"
#include "shared/pathfind/local.h"
#include "shared/pathfind/preprocess.h"
#include "shared/sim/chunk.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

#define X tile_deep_water
#define _ tile_plain

#define MAPD 3
#define MAPLEN ((MAPD * MAPD) * 256)
static char map[MAPLEN] = {
	/*               *               *              */
	"wwwwwwwwwwwwwwwwwwwwwwwwww    ++++++++++++++++++" //-----------
	"wwwwwwwwwwwwwwwwwwwwwwwwww         +++         +"
	"wwwwwwwwwwwwwwwwwwwwwwwwww    +++     + + ++++ +"
	"wwwwwwwwwwww     wwwwwwww       +  +  + ++   + +"
	"  wwwwwwwwww     wwwwwww        +  +  +      + +"
	"    wwwwwwww     wwwwwww        +  +  +++  +++ +"
	" S    wwww       wwwwww         +  +  +   +    +"
	"w     www  w     wwwww          + + + +  +     +"
	"www   w   wwwwww wwww           + + +++++      +"
	"wwww    wwwwwwww www            +              +"
	"wwwww ww  wwwwww www            +++ ++++++++++++"
	"wwwwww     wwwww ww             x + +           "
	"wwwwwww     wwww                x + +           "
	"wwwwwww     www                 x + +           "
	"wwwwwww      w                  x               "
	"wwwwww                          x               "
	"wwwww                           x               " /* - */
	"wwww                            x               "
	"wwwww                           x               "
	"wwwwww                          x               "
	"wwwwwww                         x               "
	"wwwwwwwww                       x               "
	"wwwwwwwwww               ++++++xx               "
	"wwwwwwwwwww              +                      "
	"wwwwwwwwwww       mmmmmmm+                      "
	"wwwwwwwwwww     mmmmmmmmmmm                     "
	"wwwwwwwwwww    mmmmmmmmmmmmm                    "
	"wwwwwwwwww     mmm        mm                    "
	"wwwwwwwww     mmm         mm                    "
	"wwwwwwww      mm         mmm                    "
	"wwwwwww       mm        mmm                     "
	"wwwwww         mmm     mmm                      "
	"wwwww           mm    mmm                      m" //-----------
	"www             wmm  mm                       mm"
	"ww              wwmmmm                        mm"
	"ww              wwwm    G                     mm"
	"w               www+                         mmm"
	"                www+                         mmm"
	"                www+                         mmm"
	"                www+                       mmmmm"
	"                www+                       mmmmm"
	"              xxw+++++                   mmmmmmm"
	"             x                          mmmmmmmm"
	"             x  w+++++                mmmmmmmmmm"
	"              mwwwwmmmmmmmmm        mmmmmmmmmmmm"
	"           mmmwwwwwmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
	"   mmmmmmmmmwwwwwwwmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
	" mmmmmmmmmwwwwwwwwmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
	/*               *               *              */
};

static void
parse_map(struct chunks *cnks, struct point *start, struct point *goal)
{
	uint32_t i;
	struct point p = { 0 }, cp, rp;
	struct chunk *ck;
	enum tile t;

	for (i = 0; i < MAPLEN; ++i) {
		p.x = i % (MAPD * 16);
		p.y = i / (MAPD * 16);

		cp = nearest_chunk(&p);
		rp = point_sub(&p, &cp);

		switch (map[i]) {
		case 'S':
			*start = p;
			t = tile_plain;
			break;
		case 'G':
			*goal = p;
			t = tile_plain;
			break;
		case ' ':
			t = tile_plain;
			break;
		default:
			t = tile_water;
			break;
		}


		ck = get_chunk(cnks, &cp);

		ck->tiles[rp.x][rp.y] = t;
	}
}

static void
print_map(struct chunks *cnks, struct darr *points)
{
	struct point p = { 0 }, cp, rp, *pp;
	struct chunk *ck;
	uint32_t i, k;

	for (p.y = 0; p.y < (MAPD * 16); ++p.y) {
		for (p.x = 0; p.x < (MAPD * 16); ++p.x) {
			cp = nearest_chunk(&p);
			rp = point_sub(&p, &cp);

			ck = get_chunk(cnks, &cp);

			i = (p.y * MAPD * 16) + p.x;

			map[i] = ck->tiles[rp.x][rp.y] == tile_plain ? ' ' : '#';
		}
	}

	for (i = 0; i < darr_len(points); ++i) {
		pp = darr_get(points, i);
		k = (pp->y * MAPD * 16) + pp->x;
		map[k] = '!';
	}

	for (i = 0; i < MAPLEN; ++i) {
		fputc(map[i], stderr);

		if (i && !(i % (MAPD * 16))) {
			fputc('\n', stderr);
		}
	}
	fputc('\n', stderr);

	/* fprintf(stderr, "\033[%dA%d", MAPD * 16, MAPD * 16); */
}

int
main(const int argv, const char **argc)
{
	log_init();

	uint32_t i, path;
	struct chunks cnks = { 0 };
	struct point s, g;
	struct darr *path_points = darr_init(sizeof(struct point));

	chunks_init(&cnks);

	parse_map(&cnks, &s, &g);

	ag_init_components(&cnks);

	L("(%d, %d) -> (%d, %d)", s.x, s.y, g.x, g.y);

	i = 0;
	if (hpa_start(&cnks, &s, &g, &path)) {
		while ((hpa_continue(&cnks, path, &s)) == rs_cont) {
			darr_push(path_points, &s);
			++i;
		}

		L("path found");

		L("abstract nodes visited: %ld, pathlen: %d", hash_len(cnks.ag.visited), i);

		print_map(&cnks, path_points);
	} else {
		L("path not found");
	}
}
