/*
 * Copyright (c) 2019 Thomas Burdick <thomas.burdick@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file zio_fifo_buf.h
 *
 * @brief A ZIO Buffer implementation backed by a FIFO
 *
 * Implements the zio_buf interface using a zio_fifo. This can be used by
 * hardware drivers to provide a software implementation of a FIFO.
 *
 * The expectation is that each driver instance is statically allocated
 * along with an instance of a zio_fifo_buf, sized using Kconfig options specific
 * to the driver instance, where the fifo element is driver specific and self
 * described by the drivers statically defined channel array.
 */

#ifndef ZEPHYR_INCLUDE_ZIO_FIFO_BUF_H_
#define ZEPHYR_INCLUDE_ZIO_FIFO_BUF_H_

#include <zio/zio_fifo.h>
#include <zio/zio_buf.h>

/**
 * @private
 * @brief Software FIFO implementation of a zio_buf
 */
struct z_zio_fifo_buf {
	u32_t watermark;
	u32_t length;
	struct k_poll_signal signal;
	struct zio_fifo *fifo;
};

/**
 * @brief Statically initialize an anonymous representing a zio_fifo_buf
 *
 * Statically initialize a zio_fifo_buf with a fixed number of elements of a
 * given type
 */
#define ZIO_FIFO_BUF_STATIC_INIT(name, type, pow)				\
	{									\
		.buf = {							\
			.signal = K_POLL_SIGNAL_INITIALIZER((name.buf.signal)),	\
			.fifo = &(name).fifo.zfifo				\
		},								\
		.fifo = ZIO_FIFO_STATIC_INIT((name.fifo), type, pow)		\
	}

/**
 * @brief Declare an anonymous struct type for a zio_fifo_buf
 *
 * Declare a zio_fifo_buf with a fixed number of elements of a given type
 */
#define ZIO_FIFO_BUF_DECLARE(name, type, pow)	   \
	struct {				   \
		struct z_zio_fifo_buf buf;	   \
		ZIO_FIFO_DECLARE(fifo, type, pow); \
	} name

/**
 * @brief Define a zio_fifo_buf with a specific name, type, and size
 */
#define ZIO_FIFO_BUF_DEFINE(name, type, pow)	\
	ZIO_FIFO_BUF_DECLARE(name, type, pow) =	\
		ZIO_FIFO_BUF_STATIC_INIT(name, type, pow)

/**
 * @brief Define a static zio_fifo_buf with a specific name, type, and size
 */
#define ZIO_FIFO_BUF_DEFINE_STATIC(name, type, pow) \
	static ZIO_FIFO_BUF_DEFINE(name, type, pow)


/**
 * @brief Push a datum into the fifo notifying event pollers if needed
 *
 * @param fifobuf Pointer to a "zio_fifo_buf" definition
 * @param datum Value to push into zio_fifo_buf
 */
#define zio_fifo_buf_push(fifobuf, datum)				      \
	({								      \
		int ret = 0;						      \
		if (zio_fifo_push(&(fifobuf)->fifo, datum)) {		      \
			(fifobuf)->buf.length += 1;			      \
		}							      \
		if ((fifobuf)->buf.length >= (fifobuf)->buf.watermark) {      \
			ret = k_poll_signal_raise(&(fifobuf)->buf.signal, 0); \
		}							      \
		ret;							      \
	})


extern struct zio_buf_api zio_fifo_buf_api;

#endif /* ZEPHYR_INCLUDE_ZIO_FIFO_BUF_H_ */
