/******************************************************************************
 *
 *   Copyright (C) 2011  Intel Corporation. All rights reserved.
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *****************************************************************************/

#include <linux/device.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

/* registers */
#define LSM9DS1_REG_ACT_THS          0x04
#define LSM9DS1_REG_ACT_DUR          0x05
#define LSM9DS1_REG_INT_GEN_CFG_XL   0x06
#define LSM9DS1_REG_INT_GEN_THS_X_XL 0x07
#define LSM9DS1_REG_INT_GEN_THS_Y_XL 0x08
#define LSM9DS1_REG_INT_GEN_THS_Z_XL 0x09
#define LSM9DS1_REG_INT_GEN_DUR_XL   0x0A
#define LSM9DS1_REG_REFERENCE_G      0x0B
#define LSM9DS1_REG_INT1_CTRL        0x0C
#define LSM9DS1_REG_INT2_CTRL        0x0D
#define LSM9DS1_REG_WHO_AM_I         0x0F
#define LSM9DS1_REG_CTRL_REG1_G      0x10
#define LSM9DS1_REG_CTRL_REG2_G      0x11
#define LSM9DS1_REG_CTRL_REG3_G      0x12
#define LSM9DS1_REG_ORIENT_CFG_G     0x13
#define LSM9DS1_REG_INT_GEN_SRC_G    0x14
#define LSM9DS1_REG_OUT_TEMP         0x15
#define LSM9DS1_REG_STATUS_REG       0x17
#define LSM9DS1_REG_OUT_X_G          0x18
#define LSM9DS1_REG_OUT_Y_G          0x1A
#define LSM9DS1_REG_OUT_Z_G          0x1C
#define LSM9DS1_REG_CTRL_REG4        0x1E
#define LSM9DS1_REG_CTRL_REG5_XL     0x1F
#define LSM9DS1_REG_CTRL_REG6_XL     0x20
#define LSM9DS1_REG_CTRL_REG7_XL     0x21
#define LSM9DS1_REG_CTRL_REG8        0x22
#define LSM9DS1_REG_CTRL_REG9        0x23
#define LSM9DS1_REG_CTRL_REG10       0x24
#define LSM9DS1_REG_INT_GEN_SRC_XL   0x26
#define LSM9DS1_REG_STATUS_REG_BIS   0x27
#define LSM9DS1_REG_OUT_X_XL         0x28
#define LSM9DS1_REG_OUT_Y_XL         0x2A
#define LSM9DS1_REG_OUT_Z_XL         0x2C
#define LSM9DS1_REG_FIFO_CTRL        0x2E
#define LSM9DS1_REG_FIFO_SRC         0x2F
#define LSM9DS1_REG_INT_GEN_CFG_G    0x30
#define LSM9DS1_REG_INT_GEN_THS_X_G  0x31
#define LSM9DS1_REG_INT_GEN_THS_Y_G  0x33
#define LSM9DS1_REG_INT_GEN_THS_Z_G  0x35
#define LSM9DS1_REG_INT_GEN_DUR_G    0x37
#define LSM9DS1_REG_OFFSET_X_REG_M   0x05
#define LSM9DS1_REG_OFFSET_Y_REG_M   0x07
#define LSM9DS1_REG_OFFSET_Z_REG_M   0x09
#define LSM9DS1_REG_WHO_AM_I_M       0x0F
#define LSM9DS1_REG_CTRL_REG1_M      0x20
#define LSM9DS1_REG_CTRL_REG2_M      0x21
#define LSM9DS1_REG_CTRL_REG3_M      0x22
#define LSM9DS1_REG_CTRL_REG4_M      0x23
#define LSM9DS1_REG_CTRL_REG5_M      0x24
#define LSM9DS1_REG_STATUS_REG_M     0x27
#define LSM9DS1_REG_OUT_X_M          0x28
#define LSM9DS1_REG_OUT_Y_M          0x2A
#define LSM9DS1_REG_OUT_Z_M          0x2C
#define LSM9DS1_REG_INT_CFG_M        0x30
#define LSM9DS1_REG_INT_SRC_M        0x31
#define LSM9DS1_REG_INT_THS_M        0x32

/* bits */
#define LSM9DS1_I_AM         0b01101000


/* enum { */
/*         LSM9DS0, */
/*         LSM9DS1 */
/* }; */

struct lsm9ds1_data {
	struct i2c_client *client;
};

#define LSM9DS1_CHANNEL_TEMP(reg) {	\
        .type = IIO_TEMP, \
        .address = reg,	\
	.channel = 0,	\
	.scan_type = { \
		.sign = 's', \
		.realbits = 12, \
		.storagebits = 16, \
		.endianness = IIO_LE, \
         },                                      \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW), \
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE) | \
                        BIT(IIO_CHAN_INFO_OFFSET),             \
}

#define LSM9DS1_CHANNEL_XL(reg, axis) {	\
        .type = IIO_ACCEL, \
        .address = reg,	\
	.modified = 1,	\
	.channel2 = IIO_MOD_##axis,	\
	.scan_type = { \
		.sign = 's', \
		.realbits = 16, \
		.storagebits = 16, \
		.endianness = IIO_LE, \
         },                                      \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	\
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE), \
}

#define LSM9DS1_CHANNEL_G(reg, axis) {	\
        .type = IIO_ANGL_VEL, \
        .address = reg,	\
	.modified = 1,	\
	.channel2 = IIO_MOD_##axis,	\
	.scan_type = { \
		.sign = 's', \
		.realbits = 16, \
		.storagebits = 16, \
		.endianness = IIO_LE, \
         },                                      \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	\
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE), \
}

static const struct iio_chan_spec lsm9ds1_channels[] = {
        LSM9DS1_CHANNEL_TEMP(LSM9DS1_REG_OUT_TEMP),
	LSM9DS1_CHANNEL_XL(LSM9DS1_REG_OUT_X_XL, X),
	LSM9DS1_CHANNEL_XL(LSM9DS1_REG_OUT_Y_XL, Y),
	LSM9DS1_CHANNEL_XL(LSM9DS1_REG_OUT_Z_XL, Z),
	LSM9DS1_CHANNEL_G(LSM9DS1_REG_OUT_X_G, X),
	LSM9DS1_CHANNEL_G(LSM9DS1_REG_OUT_Y_G, Y),
	LSM9DS1_CHANNEL_G(LSM9DS1_REG_OUT_Z_G, Z),
};

static int lsm9ds1_register_mask_write(struct i2c_client *client, u16 addr,
                                     u8 mask, u8 data)
{
        int ret;
        u8 tmp_data = 0;
        
        ret = i2c_smbus_read_byte_data(client, addr);
        if (ret < 0)
                return ret;

        tmp_data = ret & ~mask;
        tmp_data |= data & mask;
        return i2c_smbus_write_byte_data(client, addr, tmp_data);
}

static int lsm9ds1_register_set_bit(struct i2c_client *client, u16 addr,
                                    u8 mask)
{
        return lsm9ds1_register_mask_write(client, addr, mask, mask);
}

static int lsm9ds1_register_reset_bit(struct i2c_client *client, u16 addr,
                                      u8 mask)
{
        return lsm9ds1_register_mask_write(client, addr, 0, mask);
}

#define LSM9DS1_SW_RESET 1

static int lsm9ds1_reset(struct i2c_client *client)
{
        return lsm9ds1_register_set_bit(client, LSM9DS1_REG_CTRL_REG8,
                                        LSM9DS1_SW_RESET);
}

#define LSM9DS1_ODR_G_MASK 7 << 5
#define LSM9DS1_ODR_G_PD   0
#define LSM9DS1_ODR_G_14_9 1 << 5
#define LSM9DS1_ODR_G_59_9 2 << 5
#define LSM9DS1_ODR_G_119  3 << 5
#define LSM9DS1_ODR_G_238  4 << 5
#define LSM9DS1_ODR_G_476  5 << 5
#define LSM9DS1_ODR_G_952  6 << 5

#define LSM9DS1_ODR_XL_MASK 7 << 5
#define LSM9DS1_ODR_XL_PD   0
#define LSM9DS1_ODR_XL_10   1 << 5
#define LSM9DS1_ODR_XL_50   2 << 5
#define LSM9DS1_ODR_XL_119  3 << 5
#define LSM9DS1_ODR_XL_238  4 << 5
#define LSM9DS1_ODR_XL_476  5 << 5
#define LSM9DS1_ODR_XL_952  6 << 5

static int lsm9ds1_enable(struct i2c_client *client, bool enable)
{
        int ret;
        
        if (enable)
                return lsm9ds1_register_mask_write(client, LSM9DS1_REG_CTRL_REG1_G,
                                                   LSM9DS1_ODR_G_MASK, LSM9DS1_ODR_G_59_9);
        
        ret = lsm9ds1_register_mask_write(client, LSM9DS1_REG_CTRL_REG1_G,
                                          LSM9DS1_ODR_G_MASK, LSM9DS1_ODR_G_PD);
        if (ret < 0)
                return ret;

	return lsm9ds1_register_mask_write(client, LSM9DS1_REG_CTRL_REG6_XL,
                                           LSM9DS1_ODR_XL_MASK, LSM9DS1_ODR_XL_PD);
}


#define LSM9DS1_FS_XL_2G  0
#define LSM9DS1_FS_XL_16G 1 << 3
#define LSM9DS1_FS_XL_4G  2 << 3
#define LSM9DS1_FS_XL_8G  3 << 3
#define LSM9DS1_FS_XL_MASK 0b00011000

#define LSM9DS1_FS_XL_SCALE_2G  IIO_G_TO_M_S_2( 2000000 / S16_MAX)
#define LSM9DS1_FS_XL_SCALE_4G  IIO_G_TO_M_S_2( 4000000 / S16_MAX)
#define LSM9DS1_FS_XL_SCALE_8G  IIO_G_TO_M_S_2( 8000000 / S16_MAX)
#define LSM9DS1_FS_XL_SCALE_16G IIO_G_TO_M_S_2(16000000 / S16_MAX)

#define LSM9DS1_FS_G_245DPS  0
#define LSM9DS1_FS_G_500DPS  1 << 3
#define LSM9DS1_FS_G_2000DPS 3 << 3
#define LSM9DS1_FS_G_MASK    0b00011000

#define LSM9DS1_FS_G_SCALE_245DPS  IIO_DEGREE_TO_RAD( 245000000 / S16_MAX)
#define LSM9DS1_FS_G_SCALE_500DPS  IIO_DEGREE_TO_RAD( 500000000 / S16_MAX)
#define LSM9DS1_FS_G_SCALE_2000DPS IIO_DEGREE_TO_RAD(2000000000 / S16_MAX)

static int lsm9ds1_read_raw(struct iio_dev *indio_dev,
                            struct iio_chan_spec const *chan,
                            int *val, int *val2, long mask)
{
	struct lsm9ds1_data *data = iio_priv(indio_dev);
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = i2c_smbus_read_word_data(data->client, chan->address);
		if (ret < 0)
			return ret;
		*val = ret;
                *val2 = 0;
		return IIO_VAL_INT;
        case IIO_CHAN_INFO_SCALE:
                switch (chan->type) {
                case IIO_TEMP:
                        *val = 0;
                        *val2 = 1000000 / 16;
                        return IIO_VAL_INT_PLUS_MICRO;
                case IIO_ACCEL:
                        ret = i2c_smbus_read_byte_data(data->client,
                                                       LSM9DS1_REG_CTRL_REG6_XL);
                        if (ret < 0)
                                return ret;

                        *val = 0;
                        switch (ret & LSM9DS1_FS_XL_MASK) {
                        case LSM9DS1_FS_XL_2G:
                                *val2 = LSM9DS1_FS_XL_SCALE_2G;
                                return IIO_VAL_INT_PLUS_MICRO;
                        case LSM9DS1_FS_XL_4G:
                                *val2 = LSM9DS1_FS_XL_SCALE_4G;
                                return IIO_VAL_INT_PLUS_MICRO;
                        case LSM9DS1_FS_XL_8G:
                                *val2 = LSM9DS1_FS_XL_SCALE_8G;
                                return IIO_VAL_INT_PLUS_MICRO;
                        case LSM9DS1_FS_XL_16G:
                                *val2 = LSM9DS1_FS_XL_SCALE_16G;
                                return IIO_VAL_INT_PLUS_MICRO;
                        default:
                                dev_err(&indio_dev->dev, "unaccessible code accessed\n");
                                return -ENODEV;
                        }
                case IIO_ANGL_VEL:
                        ret = i2c_smbus_read_byte_data(data->client,
                                                       LSM9DS1_REG_CTRL_REG1_G);
                        if (ret < 0)
                                return ret;

                        *val = 0;
                        switch (ret & LSM9DS1_FS_G_MASK) {
                        case LSM9DS1_FS_G_245DPS:
                                *val2 = LSM9DS1_FS_G_SCALE_245DPS;
                                return IIO_VAL_INT_PLUS_MICRO;
                        case LSM9DS1_FS_G_500DPS:
                                *val2 = LSM9DS1_FS_G_SCALE_500DPS;
                                return IIO_VAL_INT_PLUS_MICRO;
                        case LSM9DS1_FS_G_2000DPS:
                                *val2 = LSM9DS1_FS_G_SCALE_2000DPS;
                                return IIO_VAL_INT_PLUS_MICRO;
                        default:
                                dev_err(&indio_dev->dev, "unaccessible code accessed\n");
                                return -ENODEV;
                        }
                default:
                        return -EINVAL;
                }
        case IIO_CHAN_INFO_OFFSET:
                switch (chan->type) {
                case IIO_TEMP:
                        *val = 25 * 16;
                        *val2 = 0;
                        return IIO_VAL_INT;
                default:
                        return -EINVAL;
                }
	default:
		return -EINVAL;
	}
}


static const struct iio_info lsm9ds1_info = {
	.driver_module	= THIS_MODULE,
	.read_raw	= lsm9ds1_read_raw,
};

static int lsm9ds1_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
	int ret;
	struct iio_dev *indio_dev;
	struct lsm9ds1_data *data;

	ret = i2c_smbus_read_byte_data(client, LSM9DS1_REG_WHO_AM_I);
	if (ret != LSM9DS1_I_AM)
		return (ret < 0) ? ret : -ENODEV;
        
	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	data->client = client;
	i2c_set_clientdata(client, indio_dev);

	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &lsm9ds1_info;
        indio_dev->name = "lsm9ds1";
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = lsm9ds1_channels;
        indio_dev->num_channels = ARRAY_SIZE(lsm9ds1_channels);

        ret = lsm9ds1_reset(client);
	if (ret < 0)
                return ret;

	ret = lsm9ds1_enable(client, true);
	if (ret < 0)
		return ret;

	ret = iio_device_register(indio_dev);
	if (ret < 0) {
		dev_err(&client->dev, "device_register failed\n");
		lsm9ds1_enable(client, false);
	}

	return ret;
}

static int lsm9ds1_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);

	iio_device_unregister(indio_dev);

	return lsm9ds1_enable(client, false);
}

static struct i2c_device_id lsm9ds1_i2c_ids[] = {
        { "lsm9ds1", 0 },
        { }
};

MODULE_DEVICE_TABLE(i2c, lsm9ds1_i2c_ids);

static struct i2c_driver lsm9ds1_driver = {
	.driver = {
		.name = "lsm9ds1",
		.pm = NULL,
	},
	.probe		= lsm9ds1_probe,
	.remove		= lsm9ds1_remove,
	.id_table	= lsm9ds1_i2c_ids,
};

module_i2c_driver(lsm9ds1_driver);

MODULE_AUTHOR("Jérôme Guéry <jerome.guery@gmail.com>");
MODULE_DESCRIPTION("accelerometer and gyroscope part of the IMU LSM9DS1");
MODULE_LICENSE("GPL");
