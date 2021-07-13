#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/types/ring_buffer.h"
#include "shared/util/mem.h"

/* inspired by:
 * https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
 */

void
ring_buffer_init(struct ring_buffer *rb, uint32_t item_size, uint32_t len)
{
	*rb = (struct ring_buffer) {
		.item_size = item_size,
		.mask = len - 1,
		.buf = z_malloc(item_size * len),
	};

	assert((len & rb->mask) == 0 && "ring buffer size must be a power of 2");
}

void
ring_buffer_deinit(struct ring_buffer *rb)
{
	z_free(rb->buf);
}

void *
ring_buffer_pop(struct ring_buffer *rb)
{
	void *ret = NULL;

	if (rb->read != rb->write) {
		ret = &rb->buf[(rb->read & rb->mask) * rb->item_size];
		++rb->read;
	}

	return ret;
}

bool
ring_buffer_push(struct ring_buffer *rb, void *val)
{
	bool ret = false;

	if (rb->write - rb->read != (rb->mask + 1)) {
		memcpy(&rb->buf[(rb->write & rb->mask) * rb->item_size], val, rb->item_size);
		++rb->write;
		ret = true;
	}

	return ret;
}
