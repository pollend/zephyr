/*
 * Copyright (c) 2019 Thomas Burdick <thomas.burdick@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file zio_buf.h
 *
 * @brief ZIO Buffer provides a pollable interface to a FIFO like buffer
 *
 * Device drivers implement this interface using either a software or
 * hardware FIFO.
 *
 * The format of the buffer is device dependent and thus a device must
 * implement part of the API by describing the layout of the buffer. Each
 * device is responsible for self describing the position, byte size, byte
 * order, bit precision, and bit shift each value provides in a static
 * array of struct zio_chan values.
 *
 * Changing enabled/disabled channels while using the same zio_buf isn't
 * supported for that reason.
 *
 * In addition to the data itself a 32 bit trigger timestamp corresponding to
 * k_cycle_get_32() may be placed as the last set of bytes of each datum
 * pulled.
 *
 * The buffer is pollable using k_poll where the pollable event becomes
 * available when the zio_buf contains at least the designated watermark
 * number of datums. It will cease to become pollable once it contains
 * less than the designated watermark number of datums.
 *
 * Adjusting the watermark value may or may not be implemented depending
 * on the FIFO implementation.
 *
 * The ability to poll one or more buffers allows an application designer
 * to decide how to deal with incoming streams of data from one or more
 * sensors.
 *
 * This is heavily inspired by the fantastic work in Linux's IIO subsystem,
 * with a microcontroller focused RTOS in mind.
 *
 * As an example in the case of a 9 DoF IMU sensor where each channel
 * produces a 16 bit value the buffer after each trigger without timestamps
 * would contain 18 bytes of data, 22 bytes with the 32bit cycle counter
 * timestamp.
 */

#ifndef ZEPHYR_INCLUDE_ZIO_BUF_H_
#define ZEPHYR_INCLUDE_ZIO_BUF_H_

#include <zephyr/types.h>
#include <kernel.h>

#include <zio/zio_dev.h>

struct zio_buf;

/**
 * @brief Function to pull from the zio_buf a single datum
 *
 * The size of the datum is decided by the device and its set of active
 * channels. Its expected the caller already knows the exact size of
 * sample.
 */
typedef int (*zio_buf_pull_t)(struct zio_buf *buf, void *datum);

/**
 * @brief Function to setup a poll event for the zio_buf
 *
 * The device itself decides when data becomes available and how. Then in
 * a zio application a k_poll_event is setup as a way of polling one or
 * more buffers.
 */
typedef int (*zio_buf_poll_init_t)(struct zio_buf *buf,
				   struct k_poll_event *evt);

/**
 * @brief Function to set watermark of zio_buf
 *
 * Optional function for a driver if watermark manipulation is possible
 */
typedef int (*zio_buf_set_watermark_t)(struct zio_buf *buf, u32_t watermark);

/**
 * @brief Function to get watermark of zio_buf
 *
 * Optional function for a driver to return the known watermark
 */
typedef int (*zio_buf_get_watermark_t)(struct zio_buf *buf, u32_t *watermark);

/**
 * @brief Function to get length of zio_buf
 *
 * Optional function for a driver to return the known length
 */
typedef int (*zio_buf_get_length_t)(struct zio_buf *buf, u32_t *length);


/**
 * @brief A pollable buffer interface for reading and writing to streams of data
 *
 * Most devices will want to simply use a fifo backed zio_buf but optionally
 * devices may provide a hardware fifo backed zio_buf.
 *
 * The interface should support either. In the case of a fifo backed zio_buf
 * devices should make a best effort to use DMA transfers rather than memcpy
 */
struct zio_buf_api {
	zio_buf_pull_t pull;
	zio_buf_poll_init_t poll_init;
	zio_buf_set_watermark_t set_watermark;
	zio_buf_get_watermark_t get_watermark;
	zio_buf_get_length_t get_length;
};

/**
 * @brief A pollable fifo-like buffer for reading and writing to streams of data
 *
 * An implementation should implement the api above statically optionally
 * setting a void* to its own data to be used. Each device driver may
 * implement their own zio_buf or base it on the software zio_fifo_buf
 * implementation already provided.
 */
struct zio_buf {
	bool circular : 1;
	enum overflow_type {
		ZIO_BUF_OVERFLOW_NONE,
		ZIO_BUF_OVERFLOW_FLAG,
		ZIO_BUF_OVERFLOW_COUNT,
	} overflow_type;
	u32_t overflow;
	u32_t datum_size;
	bool timestamps;
	struct device *device;
	struct zio_buf_api *buf_api;
	void *buf_data;
};

/**
 * @brief Attach a buffer to a device.
 *
 * The buffer must use a fifo which uses the same sample type as the driver.
 *
 * TODO check for that at compile time in same way if possible
 *
 * @param buf Pointer to a zio_buf struct
 * @param dev Pointer to a zephyr device which implements the zio api
 *
 * @return 0 on success, -errno on failure.
 */
int zio_buf_attach(struct zio_buf *buf, struct device *dev)
{
	const struct zio_dev_api *api = dev->driver_api;

	if (!api->attach_buf) {
		return -ENOTSUP;
	}
	return api->attach_buf(dev, buf);
}

/**
 * @brief Detach the buffer from the device
 *
 * @param buf Pointer to a zio_buf struct
 * @return 0 on success, -errno on failure.
 */
int zio_buf_detach(struct zio_buf *buf)
{
	struct device *dev = buf->device;
	const struct zio_dev_api *api = dev->driver_api;

	if (!api->detach_buf) {
		return -ENOTSUP;
	}
	return api->detach_buf(dev, buf);
}

/**
 * @brief Set the desired watermark
 *
 * Not all zio_buf implementations provide watermark manipulation as
 * many hardware backed implementations might not provide this functionality.
 *
 * @param buf Pointer to a zio_buf struct
 * @param watermark Desired watermark to cause a pollable signal to be set
 * @return 0 on success, -errno on failure.
 */
int zio_buf_set_watermark(struct zio_buf *buf, u32_t watermark)
{
	const struct zio_buf_api *api = buf->buf_api;

	if (!api->set_watermark) {
		return -ENOTSUP;
	}
	return api->set_watermark(buf, watermark);
}

/**
 * @brief Get the watermark
 *
 * @param buf Pointer to a zio_buf struct
 * @param watermark Watermark to cause a signal being set
 * @return 0 on success, -errno on failure.
 */
int zio_buf_get_watermark(struct zio_buf *buf, u32_t *watermark)
{
	const struct zio_buf_api *api = buf->buf_api;

	if (!api->get_watermark) {
		return -ENOTSUP;
	}
	return api->get_watermark(buf, watermark);
}

/**
 * @brief Get the length of the buffer
 *
 * @param buf Pointer to a zio_buf struct
 * @param length Pointer to a u32_t where length will be assigned
 * @return 0 on success, -errno on failure.
 */
int zio_buf_get_length(struct zio_buf *buf, u32_t *length)
{
	const struct zio_buf_api *api = buf->buf_api;

	if (!api->get_length) {
		return -ENOTSUP;
	}
	return api->get_length(buf, length);
}


/* TODO
 * - define helpers for accessing each channel or groups of channels in a
 *   more meaningful way from interleaved samples
 *   ie accel x,y,z gyro x,y,z, timestamp etc
 * - define helpers to wait some number of samples to show up, k_poll plus count
 *   of polls notified, i.e. zio_buf_fill()
 * - define helpers for obtaining SI unit converted values when possible
 */

#endif /* ZEPHYR_INCLUDE_ZIO_BUF_H_ */
