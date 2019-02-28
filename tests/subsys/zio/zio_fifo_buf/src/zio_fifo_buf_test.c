/*
 * Copyright (c) 2019 Thomas Burdick <thomas.burdick@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Test zio fifo buf
 */


#include <tc_util.h>
#include <stdbool.h>
#include <zephyr.h>
#include <ztest.h>
#include <zio/zio_fifo_buf.h>

struct random {
	u32_t something;
	u32_t something2;
	u8_t something3;
};

/* test static define, would be a compile error if it didn't work correctly */
ZIO_FIFO_BUF_DEFINE_STATIC(mybufforever, struct random, 8);

static void test_zio_fifo_buf_define(void)
{
	ZIO_FIFO_BUF_DEFINE(mybuf, u16_t, 8);

	/* test second define, would be a compile time error if it failed */
	ZIO_FIFO_BUF_DEFINE(mybuf2, u16_t, 8);
}

static void test_zio_fifo_buf_push(void)
{
	ZIO_FIFO_BUF_DEFINE(mybuf, u16_t, 8);
	zio_fifo_buf_push(&mybuf, 5);
	zassert_equal(zio_fifo_size(&mybuf.fifo), 256, "Unexpected size");
	zassert_equal(zio_fifo_used(&mybuf.fifo), 1, "Unexpected used");
}

static void test_zio_fifo_buf_signal(void)
{
	ZIO_FIFO_BUF_DEFINE(mybuf, u16_t, 8);
	zassert_equal(mybuf.buf.signal.signaled, 0, "Unexpected signal state");
	mybuf.buf.watermark = 1;
	zio_fifo_buf_push(&mybuf, 5);
	zassert_equal(mybuf.buf.signal.signaled, 1, "Unexpected signal state");
}


/*test case main entry*/
void test_main(void)
{
	ztest_test_suite(test_zio_fifo_buf_list,
			 ztest_unit_test(test_zio_fifo_buf_define),
			 ztest_unit_test(test_zio_fifo_buf_push),
			 ztest_unit_test(test_zio_fifo_buf_signal)
			 );
	ztest_run_test_suite(test_zio_fifo_buf_list);
}
