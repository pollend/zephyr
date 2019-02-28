/*
 * Copyright (c) 2019 Thomas Burdick <thomas.burdick@gmail.com>
 * Copyright (c) 2019 Kevin Townsend
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ZIO attribute definitions.
 */

#ifndef ZEPHYR_INCLUDE_ZIO_ATTR_H_
#define ZEPHYR_INCLUDE_ZIO_ATTR_H_

#include <zephyr/types.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ZIO attribute enum and struct definitions.
 * @defgroup zio_attributes ZIO attribute definitions
 * @ingroup zio
 * @{
 */

/**
 * @brief A type tag enum
 */
/**< Value type identifier. */
enum zio_attr_type {
	zio_attr_float,                 /**< Single-precision float tag. */
	zio_attr_double,                /**< Double-precision float tag. */
	zio_attr_bool,                  /**< 1-bit boolean tag. */
	zio_attr_u8,                    /**< Unsigned 8-bit integer tag. */
	zio_attr_u16,                   /**< Unsigned 16-bit integer tag. */
	zio_attr_u32,                   /**< Unsigned 32-bit integer tag. */
	zio_attr_u64,                   /**< Unsigned 64-bit integer tag. */
	zio_attr_s8,                    /**< Signed 8-bit integer tag. */
	zio_attr_s16,                   /**< Signed 16-bit integer tag. */
	zio_attr_s32,                   /**< Signed 32-bit integer tag. */
	zio_attr_s64,                   /**< Signed 64-bit integer tag. */
	zio_attr_str,                   /**< String tag. */
	zio_attr_ptr,                   /**< Generic pointer tag. */
};

/**
 * @brief Tag for a value of known type
 */
 #define zio_attr_tag(val) _Generic((val),			      \
				    float : zio_attr_float,	      \
				    double : zio_attr_double,	      \
				    bool : zio_attr_bool,	      \
				    unsigned char : zio_attr_u8,      \
				    unsigned short : zio_attr_u16,    \
				    unsigned long : zio_attr_u32,     \
				    unsigned long long, zio_attr_u64, \
				    signed char : zio_attr_s8,	      \
				    signed short : zio_attr_s16,      \
				    signed long : zio_attr_s32,	      \
				    signed long long : zio_attr_s64,  \
				    char * : zio_attr_str	      \
				    void * : zio_attr_ptr	      \
				    )

/**
 * @brief A type tagged variant for attributes
 */
struct zio_attr_data {
	enum zio_attr_type tag;

	/**< Assigned value. */
	union {
		float float_val;        /**< Single-precision float value. */
		double double_val;      /**< Double-precision float value. */
		u8_t bool_val : 1;      /**< 1-bit boolean value. */
		u8_t u8_val;            /**< Unsigned 8-bit integer value. */
		u16_t u16_val;          /**< Unsigned 16-bit integer value. */
		u32_t u32_val;          /**< Unsigned 32-bit integer value. */
		u64_t u64_val;          /**< Unsigned 64-bit integer value. */
		s8_t s8_val;            /**< Signed 8-bit integer value. */
		s16_t s16_val;          /**< Signed 16-bit integer value. */
		s32_t s32_val;          /**< Signed 32-bit integer value. */
		s64_t s64_val;          /**< Signed 64-bit integer value. */
		char  *str_val;         /**< String (char *) value. */
		void  *ptr_val;         /**< Generic pointer (void *) value. */
	} value;
};

/** @brief ZIO device attribute types. */
enum zio_dev_attr_type {
	/** Short presentation name. */
	DEV_ATTR_NAME,
	/** Current operating/power mode. */
	DEV_ATTR_OP_MODE,
	/** Supported operating/power modes. */
	DEV_ATTR_OP_MODE_LIST,
	/** Unique identifier for a device. */
	DEV_ATTR_UNIQUE_ID,
};

/** @brief ZIO channel attribute types. */
enum zio_chan_attr_type {
	/** Mandatory raw data attribute. */
	CHAN_ATTR_RAW_DATA = 0,
	/** SI data attribute. */
	CHAN_ATTR_SI_DATA,
	/** Short presentation name. */
	CHAN_ATTR_NAME,
	/** Current HW sampling frequency. */
	CHAN_ATTR_SAMP_FREQ,
	/** Supported HW sample frequencies. */
	CHAN_ATTR_SAMP_FREQ_LIST,
	CHAN_ATTR_OFFSET,
	/** Raw value to SI scale factor. */
	CHAN_ATTR_SCALE,
	/** Factory calibration bias. */
	CHAN_ATTR_CAL_BIAS,
	/** Factory calibration scale factor. */
	CHAN_ATTR_CAL_SCALE,
	/** Current read/event mode. */
	CHAN_ATTR_EVENT,
	/** Supported read/event modes. */
	CHAN_ATTR_EVENT_LIST,
	/** Current trigger. */
	CHAN_ATTR_TRIGGER,
	/** Supported triggers. */
	CHAN_ATTR_TRIGGER_LIST,
	/** Enable or disable the channel buffer. */
	CHAN_ATTR_BUF_ENABLED,
	/** Indicate if channel data is available. */
	CHAN_ATTR_DATA_AVAIL,
};

/** Device attribute record. */
struct zio_dev_attr {
	/** Index for this specific attribute. Assigned when bound to device. */
	u8_t idx;
	/** Attribute identifier. */
	enum zio_dev_attr_type attr_type;
	/** Attribute data type and value. */
	struct zio_attr_data data;
	/** Whether this attribute is enabled (1) or disabled (0). */
	u8_t enabled : 1;
};

/** Channel attribute record. */
struct zio_chan_attr {
	/** Index for this specific attribute.
	 * Assigned when bound to channel.
	 */
	u8_t idx;
	/** Primary attribute type identifier. */
	enum zio_chan_attr_type attr_type;
	/**
	 * Secondary attribute type identifier. Used to distinguish multiple
	 * instances of the same 'attr_type' in the channel.
	 */
	enum zio_chan_attr_type attr_subtype;
	/** Attribute data type and value. */
	struct zio_attr_data data;
	/** Whether this attribute is enabled (1) or disabled (0). */
	u8_t enabled : 1;
	/** Data tick counter. Increments every time 'data' is updated. */
	u32_t tick;
};

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  /* ZEPHYR_INCLUDE_ZIO_ATTR_H_ */
