/*
 * Copyright (c) 2019  Thomas Burdick <thomas.burdick@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_ZIO_SYNTH_H_
#define ZEPHYR_INCLUDE_DRIVERS_ZIO_SYNTH_H_

#include <zio.h>

/* channel type(s) */
#define SYNTH_AUDIO_TYPE (ZIO_CHAN_TYPES + 1)

/* channel attribute type(s) */
#define SYNTH_FREQUENCY (ZIO_ATTR_TYPES + 1)
#define SYNTH_PHASE (ZIO_ATTR_TYPES + 2)


/* datum type */
struct synth_datum {
	s16_t samples[2];
};

#endif

