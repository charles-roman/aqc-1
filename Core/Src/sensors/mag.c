/*
 * mag.c
 *
 *  Created on: Nov 25, 2023
 *      Author: charlieroman
 */

#include <string.h>
#include "main.h"
#include "mag.h"
#include "../common/maths.h"
#include "../common/time.h"
#include "../drivers/lis3mdl_reg.h"

static stmdev_ctx_t dev_ctx;

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  if (handle == &hi2c1)
  {
    /* Write multiple command */
    reg |= 0x80;
    HAL_I2C_Mem_Write(handle, LIS3MDL_I2C_ADD_L, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  }
  return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  if (handle == &hi2c1)
  {
    /* Read multiple command */
    reg |= 0x80;
    HAL_I2C_Mem_Read(handle, LIS3MDL_I2C_ADD_L, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  }
  return 0;
}

/**
  * @brief mag configuration & setup
  *
  * @param  mag		pointer to sensor_3d struct
  * @retval None
  */
void lis3mdl_setup(sensor_3d *mag)
{
	uint8_t whoamI, rst;

	/* Initialize mems driver interface */
	dev_ctx.write_reg = platform_write;
	dev_ctx.read_reg = platform_read;
	dev_ctx.handle = &hi2c1;

	/* Check device ID */
	lis3mdl_device_id_get(&dev_ctx, &whoamI);
	if (whoamI != LIS3MDL_ID)
		while(1); /*manage here device not found */

	/* Restore default configuration */
	lis3mdl_reset_set(&dev_ctx, PROPERTY_ENABLE);
	do {
		lis3mdl_reset_get(&dev_ctx, &rst);
	} while (rst);

	/* Enable Block Data Update */
	lis3mdl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	/* Set Output Data Rate */
	lis3mdl_data_rate_set(&dev_ctx, LIS3MDL_MP_560Hz);

	/* Set full scale */
	lis3mdl_full_scale_set(&dev_ctx, LIS3MDL_4_GAUSS);

	/* Enable temperature sensor */
	lis3mdl_temperature_meas_set(&dev_ctx, PROPERTY_ENABLE);

	/* Set device in continuos mode */
	lis3mdl_operating_mode_set(&dev_ctx, LIS3MDL_CONTINUOUS_MODE);

}

/**
  * @brief gets calibrated mag measurements
  *
  * @param  mag		pointer to (mag) device struct
  * @retval None
  */
static void get_calibrated_measurements(sensor_3d *mag)
{
	float x,y,z;

	/* MAG CALIBRATION CORRECTIONS */
	x = mag->x - MAG_XBIASOFFSET_mG;
	y = mag->y - MAG_YBIASOFFSET_mG;
	z = mag->z - MAG_ZBIASOFFSET_mG;

	mag->x = (S11_mag)*x + (S12_mag)*y + (S13_mag)*z;
	mag->y = (S21_mag)*x + (S22_mag)*y + (S23_mag)*z;
	mag->z = (S31_mag)*x + (S32_mag)*y + (S33_mag)*z;
}

/*
 * @brief	get MAG data in engineering units
 *
 * @param	ctx		mems driver interface(ptr)
 *
 */
static void lis3mdl_read_data(sensor_3d *mag)
{
    //uint8_t reg;

      /* Read output only if new value is available */
    //lis3mdl_mag_data_ready_get(&dev_ctx, &reg);
    //if (reg)
    //{
		/* Read magnetic field data */
		memset(mag->raw.u8bit, 0x00, 3*sizeof(int16_t));
		lis3mdl_magnetic_raw_get(&dev_ctx, mag->raw.u8bit);
		mag->x = 1000 * LIS3MDL_FROM_FS_4G_TO_G(mag->raw.i16bit[0]);
		mag->y = 1000 * LIS3MDL_FROM_FS_4G_TO_G(mag->raw.i16bit[1]);
		mag->z = 1000 * LIS3MDL_FROM_FS_4G_TO_G(mag->raw.i16bit[2]);
	//}
}

/**
  * @brief stores mag read, calibrate, and filter functions
  *
  * @param  mag		pointer to (mag) device struct
  * @retval None
  */
void read_mag_data(sensor_3d *mag)
{
	/* Update Timestep */
	uint32_t timestamp, timestep;
	static uint32_t prev_timestamp;

	timestamp = get_timestamp();
	timestep = timestamp - prev_timestamp;

	mag->timestep = USEC_TO_SEC((float)timestep);
	prev_timestamp = timestamp;

	/* Read MAG Data */
	lis3mdl_read_data(mag);

	/* Get Calibrated IMU Measurements */
	//get_calibrated_measurements(mag);
}
