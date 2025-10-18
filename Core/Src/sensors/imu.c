/*
 * imu.c
 *
 *  Created on: Oct 14, 2023
 *      Author: charlieroman
 */

#include <string.h>
#include "main.h"
#include "sensors/imu/imu.h"
#include "common/maths.h"
#include "common/time.h"
#include "lsm6dsox_reg.h"

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
	  HAL_I2C_Mem_Write(handle, LSM6DSOX_I2C_ADD_L, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
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
	  HAL_I2C_Mem_Read(handle, LSM6DSOX_I2C_ADD_L, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  }
  return 0;
}

//static void lsm6dsox_auto_calibrate(device *imu, uint8_t enable)
//{
//	if (enable)
//	{
//		int len = 100;
//		float gyro_x[len], gyro_y[len], gyro_z[len];
//		for (int i = 0; i < len; i++)
//		{
//			lsm6dsox_read_data(imu);
//			gyro_x[i] = imu->gyro.x;
//			gyro_y[i] = imu->gyro.y;
//			gyro_z[i] = imu->gyro.z;
//			delay(3);
//		}
//
//		imu->gyro.xbiasoffset = avgf(gyro_x, len);
//		imu->gyro.ybiasoffset = avgf(gyro_y, len);
//		imu->gyro.zbiasoffset = avgf(gyro_z, len);
//	}
//}

/**
  * @brief imu configuration & setup
  *
  * @param  imu		pointer to (imu) device struct
  * @retval None
  */
void lsm6dsox_setup(device *imu)
{

	uint8_t whoamI, rst;

	/* Initialize mems driver interface */
	dev_ctx.write_reg = platform_write;
	dev_ctx.read_reg = platform_read;
	dev_ctx.handle = &hi2c1;

	/* Wait sensor boot time */
	delay(10);

	/* Check device ID */
	lsm6dsox_device_id_get(&dev_ctx, &whoamI);
	if (whoamI != LSM6DSOX_ID)
	  while(1);

	/* Restore default configuration */
	lsm6dsox_reset_set(&dev_ctx, PROPERTY_ENABLE);
	do {
	  lsm6dsox_reset_get(&dev_ctx, &rst);
	} while (rst);

	/* Disable I3C interface */
	lsm6dsox_i3c_disable_set(&dev_ctx, LSM6DSOX_I3C_DISABLE);

	/* Enable Block Data Update */
	lsm6dsox_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	/* Set Power Mode */
	lsm6dsox_xl_power_mode_set(&dev_ctx, LSM6DSOX_HIGH_PERFORMANCE_MD);
	lsm6dsox_gy_power_mode_set(&dev_ctx, LSM6DSOX_GY_HIGH_PERFORMANCE);

	/* Set Output Data Rate */
	lsm6dsox_xl_data_rate_set(&dev_ctx, LSM6DSOX_XL_ODR_417Hz);
	lsm6dsox_gy_data_rate_set(&dev_ctx, LSM6DSOX_GY_ODR_417Hz);

	/* Set full scale */
	lsm6dsox_xl_full_scale_set(&dev_ctx, LSM6DSOX_2g);
	lsm6dsox_gy_full_scale_set(&dev_ctx, LSM6DSOX_2000dps);

	/* Enable timestamp */
	//  lsm6dsox_timestamp_set(&dev_ctx, PROPERTY_ENABLE);

	/*
	 * Configure filtering chain(No aux interface)
	 *
	 * Accelerometer - LPF1 + LPF2 path
	 */
	lsm6dsox_xl_hp_path_on_out_set(&dev_ctx, LSM6DSOX_LP_ODR_DIV_10);
	lsm6dsox_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);

	/* Set IMU Calibration Values */
	//delay(100);	// wait for polling
	//lsm6dsox_auto_calibrate(imu, PROPERTY_DISABLE);

	//	lsm6dsox_xl_usr_offset_x_set(ctx, &offset);
	//	lsm6dsox_xl_usr_offset_y_set(ctx, &offset);
	//	lsm6dsox_xl_usr_offset_z_set(ctx, &offset);

}

/**
  * @brief 3d sensor lowpass filter
  *
  * @param  sens	pointer to sensor_3d struct
  * @param  Wc		cutoff frequency
  *
  * @retval None
  */
static void lowpass_filt(sensor_3d *sens, float Wc)
{
	static float x, y, z, x_filtered, y_filtered, z_filtered, T;

	/* LPF Difference Equation */
	T = sens->timestep;

	float A = (2 - T*Wc)/(2 + T*Wc);
	float B = (T*Wc)/(2 + T*Wc);
	sens->x_filtered = A*(x_filtered) + B*(sens->x + x);
	sens->y_filtered = A*(y_filtered) + B*(sens->y + y);
	sens->z_filtered = A*(z_filtered) + B*(sens->z + z);

	/* Update Previous Measurements */
	x = sens->x;
	y = sens->y;
	z = sens->z;
	x_filtered = sens->x_filtered;
	y_filtered = sens->y_filtered;
	z_filtered = sens->z_filtered;
}

/**
  * @brief gets calibrated imu measurements
  *
  * @param  imu		pointer to (imu) device struct
  * @retval None
  */
static void get_calibrated_measurements(device *imu)
{
	float x,y,z;

	/* GYRO CALIBRATION CORRECTIONS */
	imu->gyro.x -= GYRO_XBIASOFFSET_MDPS;
	imu->gyro.y -= GYRO_YBIASOFFSET_MDPS;
	imu->gyro.z -= GYRO_ZBIASOFFSET_MDPS;

	/* ACCEL CALIBRATION CORRECTIONS */
	x = imu->accel.x - ACCEL_XBIASOFFSET_MG;
	y = imu->accel.y - ACCEL_YBIASOFFSET_MG;
	z = imu->accel.z - ACCEL_ZBIASOFFSET_MG;

	imu->accel.x = x;
	imu->accel.x = y;
	imu->accel.x = z;

//	imu->accel.x = (S11_xl)*x + (S12_xl)*y + (S13_xl)*z;
//	imu->accel.y = (S21_xl)*x + (S22_xl)*y + (S23_xl)*z;
//	imu->accel.z = (S31_xl)*x + (S32_xl)*y + (S33_xl)*z;

}

/*
 * @brief	get IMU data in engineering units
 *
 * @param	ctx		mems driver interface(ptr)
 *
 */
static void lsm6dsox_read_data(device *imu)
{
	// lsm6dsox_reg_t reg;

    /* Read output only if new value is available */
    // lsm6dsox_status_reg_get(&dev_ctx_imu, &reg.status_reg);

    // if (reg.status_reg.xlda && reg.status_reg.gda)
    //  {
		/* Read acceleration field data */
		memset(imu->accel.raw.u8bit, 0x00, 3 * sizeof(int16_t));
		lsm6dsox_acceleration_raw_get(&dev_ctx, imu->accel.raw.u8bit);
		imu->accel.x = lsm6dsox_from_fs2_to_mg(imu->accel.raw.i16bit[0]);
		imu->accel.y = lsm6dsox_from_fs2_to_mg(imu->accel.raw.i16bit[1]);
		imu->accel.z = lsm6dsox_from_fs2_to_mg(imu->accel.raw.i16bit[2]);

		/* Read angular rate field data */
		memset(imu->gyro.raw.u8bit, 0x00, 3 * sizeof(int16_t));
		lsm6dsox_angular_rate_raw_get(&dev_ctx, imu->gyro.raw.u8bit);
		imu->gyro.x = lsm6dsox_from_fs2000_to_mdps(imu->gyro.raw.i16bit[0]);
		imu->gyro.y = lsm6dsox_from_fs2000_to_mdps(imu->gyro.raw.i16bit[1]);
		imu->gyro.z = lsm6dsox_from_fs2000_to_mdps(imu->gyro.raw.i16bit[2]);

    // }
    //  uint8_t reg;
    //	lsm6dsox_temp_flag_data_ready_get(ctx, &reg); || reg.status_reg.tda ^
    //  if (reg)
    //  {
    //  /* Read temperature data */
    //  memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
    //  lsm6dsox_temperature_raw_get(ctx, data_raw_temperature.u8bit);
    //  temperature_degC = lsm6dsox_from_lsb_to_celsius(data_raw_temperature.i16bit);
    //  }
}

/**
  * @brief stores imu read, calibrate, and filter functions
  *
  * @param  imu		pointer to (imu) device struct
  * @retval None
  */
void read_imu_data(device *imu)
{
	/* Update Timestep */
	uint32_t timestamp, timestep;
	static uint32_t prev_timestamp;

	timestamp = get_timestamp();
	timestep = timestamp - prev_timestamp;

	imu->timestep = USEC_TO_SEC((float)timestep);
	imu->gyro.timestep = USEC_TO_SEC((float)timestep);
	imu->accel.timestep = USEC_TO_SEC((float)timestep);
	prev_timestamp = timestamp;

	/* Read XL + GYRO Data */
	lsm6dsox_read_data(imu);

	/* Get Calibrated IMU Measurements */
	get_calibrated_measurements(imu);

	/* Filter Measurements (OPTIONAL) */
	//lowpass_filt(&imu->gyro, GYRO_LPF_CUTOFF_FREQ);
}
