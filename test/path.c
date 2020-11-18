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
	"wwwwwww     wwww                x + +       G   "
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
	"ww              wwwm                          mm"
	"w               www+                         mmm"
	"                www+                         mmm"
	"                www+                         mmm"
	"                www+                       mmmmm"
	"                www+                       mmmmm"
	"              xxw+++++                   mmmmmmm"
	"                                        mmmmmmmm"
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

struct path_point {
	struct point pos;
	char c;
};

static void
print_map(struct chunks *cnks, struct darr *points)
{
	struct point p = { 0 }, cp, rp;
	struct path_point *pp;
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
		k = (pp->pos.y * MAPD * 16) + pp->pos.x;
		map[k] = pp->c;
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

static void
update_tile(struct chunks *cnks, enum tile t, struct point p)
{
	struct point cp = nearest_chunk(&p), rp = point_sub(&p, &cp);
	struct chunk *ck = hdarr_get(&cnks->hd, &cp);

	ck->tiles[rp.x][rp.y] = t;

	hpa_dirty_point(cnks, &p);
}

int
main(const int argv, const char **argc)
{
	log_init();

	uint32_t i, path;
	struct chunks cnks = { 0 };
	struct point s, g;
	struct darr path_points = { 0 };
	darr_init(&path_points, sizeof(struct path_point));

	chunks_init(&cnks);

	parse_map(&cnks, &s, &g);

	ag_init_components(&cnks);

	L("(%d, %d) -> (%d, %d)", s.x, s.y, g.x, g.y);

	enum result rs;

	i = 0;
	char c = '?';

	if (hpa_start(&cnks, &s, &g, &path)) {
		while ((rs = hpa_continue(&cnks, path, &s)) == rs_cont) {
			struct path_point pp = { .pos = s, .c = c };

			darr_push(&path_points, &pp);

			++i;

			if (i == 54) {
				update_tile(&cnks, tile_water, (struct point){ 32, 1 });
				c = '!';
			}

			hpa_clean(&cnks);
		}

		if (rs == rs_done) {
			L("path found");

			L("abstract nodes visited: %ld, pathlen: %d", cnks.ag.visited.len, i);

			print_map(&cnks, &path_points);
		} else {
			L("path not found");
			return 1;
		}
	} else {
		L("path not found");
		return 1;
	}
}
