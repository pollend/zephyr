/*
 * Copyright (c) 2019 Thomas Burdick <thomas.burdick@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zio.h>
#include <init.h>
#include <misc/__assert.h>
#include <misc/byteorder.h>
#include <sensor.h>
#include <string.h>
#include <logging/log.h>
#include <counter.h>
#include <sys_clock.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <drivers/zio/synth.h>

#define LOG_LEVEL CONFIG_SENSOR_LOG_LEVEL
LOG_MODULE_REGISTER(SYNTH);


ZIO_DEVICE_ATTR_SET(synth,sample_rate) {
	struct synth_data *drv_data = dev->driver_data;
	u32_t sample_rate = 0;
	if (zio_variant_unwrap(val, sample_rate) != 0) {
		return -EINVAL;
	}
	drv_data->sample_rate = sample_rate;
	return 0;
}

ZIO_DEVICE_ATTR_GET(synth,sample_rate) {
	struct synth_data *drv_data = dev->driver_data;
	*var = zio_variant_wrap(drv_data->sample_rate);
	return 0;
}

ZIO_CHANNEL_ATTR_SET(synth,frequency) {
	u32_t sample_frequency = 0;
	if(zio_variant_unwrap(val, sample_frequency) != 0){
		return -EINVAL;
	}
	struct synth_data *drv_data = dev->driver_data;
	drv_data->frequencies[chan_idx] = sample_frequency;
	return 0;
}

ZIO_CHANNEL_ATTR_GET(synth,frequency) {
	struct synth_data *drv_data = dev->driver_data;
	*var = zio_variant_wrap(drv_data->frequencies[chan_idx]);
	return 0;
}


ZIO_CHANNEL_ATTR_GET(synth,phase) {
	struct synth_data *drv_data = dev->driver_data;
	*var = zio_variant_wrap(drv_data->phases[chan_idx]);
	return 0;
}

ZIO_CHANNEL_ATTR_SET(synth,phase) {
	u32_t sample_frequency = 0;
	if(zio_variant_unwrap(val, sample_frequency) != 0){
		return -EINVAL;
	}
	struct synth_data *drv_data = dev->driver_data;
	drv_data->phases[chan_idx] = sample_frequency;
	return 0;
}

static const struct zio_device_attr_desc dev_attr_descs[1] = {
	{
		.type = ZIO_SAMPLE_RATE,
		.data_type = zio_variant_float,
		.get_attr = synth_sample_rate_get,
		.set_attr = synth_sample_rate_set
	}
};

static const struct zio_channel_attr_desc chans_attr_descs[2] = {
	{
		.type = SYNTH_FREQUENCY,
		.data_type = zio_variant_float,
		.get_attr = synth_frequency_get,
		.set_attr = synth_frequency_set
	},
	{
		.type = SYNTH_PHASE,
		.data_type = zio_variant_float,
		.get_attr = synth_phase_get,
		.set_attr = synth_phase_set
	},	
}

static const struct zio_chan_desc synth_chans[2] = {
	{
		.name = "Left",
		.type = SYNTH_AUDIO_TYPE,
		.bit_width = 16,
		.byte_size = 2,
		.byte_order = ZIO_BYTEORDER_ARCH,
		.sign_bit = ZIO_SIGN_MSB,
		.attributes = &chans_attr_descs,
		.attribute_length = 2 
	},
	{
		.name = "Right",
		.type = SYNTH_AUDIO_TYPE,
		.bit_width = 16,
		.byte_size = 2,
		.byte_order = ZIO_BYTEORDER_ARCH,
		.sign_bit = ZIO_SIGN_MSB,
		.attributes = &chans_attr_descs,
		.attribute_length = 2
	}
};

static struct synth_data {
	u32_t last_timestamp;
	u32_t t; /* samples index */
	struct device *counter;
	u32_t sample_rate;
	float frequencies[2];
	float phases[2];
	ZIO_FIFO_BUF_DECLARE(fifo, struct synth_datum, CONFIG_SYNTH_FIFO_SIZE);
}
synth_data = {
	.last_timestamp = 0,
	.t = 0,
	.sample_rate = (float)CONFIG_SYNTH_SAMPLE_RATE,
	.frequencies = {
		(float)(CONFIG_SYNTH_0_FREQ),
		(float)(CONFIG_SYNTH_1_FREQ)
	},
	.phases = {
		(float)(CONFIG_SYNTH_0_PHASE),
		(float)(CONFIG_SYNTH_1_PHASE)
	},
	.fifo = ZIO_FIFO_BUF_INITIALIZER(synth_data.fifo, struct synth_datum, CONFIG_SYNTH_FIFO_SIZE),
};


static int synth_generate(struct device *dev, u32_t n)
{
	struct synth_data *drv_data = dev->driver_data;
	float sample_rate = drv_data->sample_rate;
	struct synth_datum datum;

	for (u32_t i = 0; i < n; i++) {
		for (int j = 0; j < 2; j++) {
			float freq = drv_data->frequencies[j];
			float chan_phase = drv_data->phases[j];
			float phase = (M_PI*chan_phase)/180.0;
			float sample = sin(2.0*M_PI*(freq/sample_rate)*drv_data->t + phase);

			datum.samples[j] = (s16_t)(sample*(float)(SHRT_MAX));
		}
		zio_fifo_buf_push(&drv_data->fifo, datum);
		drv_data->t += 1;
	}
	return 0;
}

static int synth_trigger(struct device *dev)
{
	struct synth_data *drv_data = dev->driver_data;

	/* determine number of samples to generate */
	u32_t now = k_cycle_get_32();
	u32_t tstamp_diff = now - drv_data->last_timestamp;
	u64_t tdiff_ns = SYS_CLOCK_HW_CYCLES_TO_NS64(tstamp_diff);

	/* keep it simple, do all this in floats for simplicity, no speed */
	float tdiff_s = (float)tdiff_ns/1000000.0;
	float n_gen_f = drv_data->sample_rate/tdiff_s;
	u32_t n_gen = round(n_gen_f);

	drv_data->last_timestamp = now;
	return synth_generate(dev, n_gen);
}

static int synth_attach_buf(struct device *dev, struct zio_buf *buf)
{
	struct synth_data *drv_data = dev->driver_data;

	return zio_fifo_buf_attach(&drv_data->fifo, buf);
}

static int synth_detach_buf(struct device *dev)
{
	struct synth_data *drv_data = dev->driver_data;

	return zio_fifo_buf_detach(&drv_data->fifo);
}

static const struct zio_dev_api synth_driver_api = {
	.channel_length = 2,
	.channels = &synth_chans,

	.device_attributes_length = 1,
	.device_attributes = &dev_attr_descs,

	.trigger = synth_trigger,
	.attach_buf = synth_attach_buf,
	.detach_buf = synth_detach_buf
};

int synth_init(struct device *dev)
{
	/* statically initialized, so nothing to do yet but set the timestamp
	 * and add an appropriate counter callback to generate samples
	 */
	struct synth_data *drv_data = dev->driver_data;

	drv_data->last_timestamp = k_cycle_get_32();
	return 0;
}

static struct synth_data synth_data;

DEVICE_AND_API_INIT(synth, "SYNTH", synth_init,
		    &synth_data, NULL, POST_KERNEL,
		    CONFIG_ZIO_INIT_PRIORITY, &synth_driver_api);
