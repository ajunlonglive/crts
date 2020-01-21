#include <stdlib.h>
#include <string.h>

#include "container.h"
#include "window.h"

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

	dc->_root = win_init(NULL);
	dc->_root->main_win_pct = 0.8;

	dc->root.world = win_init(dc->_root);
	dc->root._info = win_init(dc->_root);
	dc->root._info->main_win_pct = 0.7;
	dc->root._info->split = 1;

	dc->root.info.l = win_init(dc->root._info);
	dc->root.info.r = win_init(dc->root._info);
}
