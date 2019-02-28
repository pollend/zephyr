/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zio/zio_dev.h>
#include <init.h>
#include <misc/__assert.h>
#include <misc/byteorder.h>
#include <sensor.h>
#include <string.h>
#include <logging/log.h>

#include <drivers/zio/synth.h>

#define LOG_LEVEL CONFIG_SENSOR_LOG_LEVEL
LOG_MODULE_REGISTER(SYNTH);

static const struct zio_dev_api synth_driver_api = {
};

int synth_init(struct device *dev)
{
	return 0;
}

struct synth_data {
	u32_t last_sample;
} synth_driver;

DEVICE_AND_API_INIT(synth, "SYNTH", synth_init,
		    &synth_driver, NULL, POST_KERNEL,
		    CONFIG_ZIO_INIT_PRIORITY, &synth_driver_api);
