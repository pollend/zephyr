/* icm20948.c - Driver for Bosch BMP280 temperature and pressure sensor */

/*
 * Copyright (c) 2016, 2017 Intel Corporation
 * Copyright (c) 2017 IpTronix S.r.l.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zio.h>
#include <gpio.h>
#include <misc/byteorder.h>
#include <misc/__assert.h>

#ifdef DT_TDK_ICM20948_BUS_I2C
#include <i2c.h>
#elif defined DT_TDK_ICM20948_BUS_SPI
#include <spi.h>
#endif

#include <drivers/zio/icm20948.h>

#define LOG_LEVEL CONFIG_SENSOR_LOG_LEVEL
LOG_MODULE_REGISTER(ICM20948);

static const struct zio_dev_chan icm_chans[] = {
	{
		.name = "ACCEL_X",
		.type = ICM20948_COORD_TYPE,
		.bit_width = 16,
		.byte_size = 2,
		.byte_order = ZIO_BYTEORDER_ARCH,
		.sign_bit = ZIO_SIGN_MSB,
	},
	{
		.name = "ACCEL_Y",
		.type = ICM20948_COORD_TYPE,
		.bit_width = 16,
		.byte_size = 2,
		.byte_order = ZIO_BYTEORDER_ARCH,
		.sign_bit = ZIO_SIGN_MSB,
	},
	{
		.name = "ACCEL_Z",
		.type = ICM20948_COORD_TYPE,
		.bit_width = 16,
		.byte_size = 2,
		.byte_order = ZIO_BYTEORDER_ARCH,
		.sign_bit = ZIO_SIGN_MSB,
	},
	{
		.name = "GYRO_X",
		.type = ICM20948_COORD_TYPE,
		.bit_width = 16,
		.byte_size = 2,
		.byte_order = ZIO_BYTEORDER_ARCH,
		.sign_bit = ZIO_SIGN_MSB,
	},
	{
		.name = "GYRO_Y",
		.type = ICM20948_COORD_TYPE,
		.bit_width = 16,
		.byte_size = 2,
		.byte_order = ZIO_BYTEORDER_ARCH,
		.sign_bit = ZIO_SIGN_MSB,
	},
	{
		.name = "GYRO_Z",
		.type = ICM20948_COORD_TYPE,
		.bit_width = 16,
		.byte_size = 2,
		.byte_order = ZIO_BYTEORDER_ARCH,
		.sign_bit = ZIO_SIGN_MSB,
	}
};

static struct icm_20948_data {
	// bus and hardware access function
	struct device *bus;
	struct icm20948_tf *hw_tf;

#if defined(DT_TDK_ICM20948_0_CS_GPIO_CONTROLLER)
	struct spi_cs_control cs_ctrl;
#endif

	u8_t bank; // selected bank
	struct zio_attr dev_attrs[1];

	ZIO_FIFO_BUF_DECLARE(fifo, struct icm20948_datum,
			     CONFIG_SYNTH_FIFO_SIZE);
}

icm_20948_data = { 
    .dev_attrs = { 
        { 
            .type = ICM20948_ACCEL_FS_TYPE,
            .data = zio_variant_u8(ICM20948_ACCEL_FS_DEFAULT)
        },
        { 
            .type = ICM20948_GYRO_FS_TYPE,
            .data = zio_variant_u8(ICM20948_GYRO_FS_DEFAULT) 
        } 
    } 
}

static int icm20948_get_chans(struct device *dev,
			      const struct zio_dev_chan **chans,
			      u32_t *num_chans)
{
	*chans = icm_chans;
	*num_chans = sizeof(icm_chans);
	return 0;
}

int icm20948_init(struct device *dev)
{
#if defined(DT_TDK_ICM20948_BUS_SPI)
	icm20948_spi_init(dev);
#elif defined(DT_TDK_ICM20948_BUS_I2C)
	icm20948_i2c_init(dev);
#else
#error "BUS MACRO NOT DEFINED IN DTS"
#endif

	u8_t tmp;
	if (data->hw_tf->read_reg(data, ICM20948_REG_WHO_AM_I, &tmp)) {
		LOG_ERR("Failed to read chip ID");
		return -EIO;
	}

	if (tmp != ICM20948_WHO_AM_I) {
		LOG_ERR("Invalid Chip ID");
	}
}

static int icm20948_set_attr(struct device *dev, const u32_t attr_idx, 
                const struct zio_variant val)
{
}

static int icm20948_get_attr(struct device *dev, u32_t attr_idx,
                struct zio_variant *var)
{
    
}

static int icm20948_get_attrs(struct device *dev, struct zio_attr **attrs,
		u32_t *num_attrs)
{
}


static int icm20948_trigger(struct device *dev)
{

}


static int icm20948_attach_buf(struct device *dev, struct zio_buf *buf)
{
	struct synth_data *drv_data = dev->driver_data;
	return zio_fifo_buf_attach(&drv_data->fifo, buf);
}

static int icm20948_detach_buf(struct device *dev)
{
	struct synth_data *drv_data = dev->driver_data;

	return zio_fifo_buf_detach(&drv_data->fifo);
}

static const struct zio_dev_api icm_20948_driver_api = {
	.set_attr = icm20948_set_attr,
	.get_attr = icm20948_get_attr,
	.get_attrs = icm20948_get_attrs,
	.get_chans = icm20948_get_chans,
	.trigger = icm20948_trigger,
	.attach_buf = icm20948_attach_buf,
	.detach_buf = icm20948_detach_buf
};

DEVICE_AND_API_INIT(icm20948, "ICM_20948", icm20948_init, &icm20948_data, NULL,
		    POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
		    &icm_20948_driver_api);
