#include <stdlib.h>
#include <string.h>

#include "client/ui/ncurses/container.h"
#include "client/ui/ncurses/window.h"
#include "shared/util/log.h"

/* display container layout:
 *
 * +--------------------------+
 * |world                     |
 * |                          |
 * |                          |
 * |                          |
 * |                          |
 * |                          |
 * |                          |
 * +-------------------+------+ < 80 %
 * |info l             |info r|
 * +-------------------+------+
 *                     ^ 70 %
 */

void
dc_init(struct display_container *dc)
{
	memset(dc, 0, sizeof(struct display_container));

	dc->_root = win_create(NULL);
	dc->_root->split_pct = 0.9;

	dc->root.world = win_create(dc->_root);
	dc->root._info = win_create(dc->_root);
	dc->root._info->split_pct = 0.7;
	dc->root._info->split = ws_vertical;

	dc->root.info.l = win_create(dc->root._info);
	dc->root.info.r = win_create(dc->root._info);

	term_commit_layout();
}
