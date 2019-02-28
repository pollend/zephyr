/*
 * Copyright (C) 2019 Thomas Burdick <thomas.burdick@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zio/zio_fifo_buf.h>


static inline void z_zio_fifo_buf_clear(z_zio_fifo_buf *fifo_buf)
{
	k_poll_signal_reset(&fifo_buf->signal);
}

static int zio_fifo_buf_pull(struct zio_buf *buf, void *datum)
{
	if (buf->length <= 0) {
		return 0;
	}

	z_zio_fifo_buf *fifo_buf = buf->impl_data;
	struct zio_fifo *fifo = fifo_buf->fifo;

	if (z_zio_fifo_pull(fifo, datum)) {
		buf->length -= 1;
		if (buf->length == 0) {
			z_zio_fifo_buf_clear(buf_fifo);
		}
	}
}

static int zio_fifo_buf_poll_init(struct zio_buf *buf, struct k_poll_event *evt)
{
	z_zio_fifo_buf *fifo_buf = buf->impl_data;

	k_poll_event_init(K_POLL_TYPE_SIGNAL,
			  K_POLL_MODE_NOTIFY_ONLY,
			  &fifo_buf->signal);
}

static int zio_fifo_buf_set_watermark(struct zio_buf *buf, u32_t watermark)
{
	z_zio_fifo_buf *fifo_buf = buf->impl_data;
	struct zio_fifo *fifo = fifo_buf->fifo;

	if (watermark > _zio_fifo_size(fifo)) {
		return -EINVAL;
	}
	buf->watermark = watermark;
	if (buf->length > buf->watermark) {
		_zio_fifo_buf_notify(buf);
	}
	return 0;
}

struct zio_buf_api zio_fifo_buf_api = {
	.pull = zio_fifo_buf_pull,
	.poll_init = zio_fifo_buf_poll_init,
	.set_watermark = zio_fifo_buf_set_watermark,
};
