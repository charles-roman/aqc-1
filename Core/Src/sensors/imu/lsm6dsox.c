/*
 * lsm6dsox.c
 *
 *  Created on: Oct 27, 2025
 *      Author: charlieroman
 */

#include <string.h>
#include "sensors/imu/lsm6dsox.h"
#include "lsm6dsox_reg.h"

/*
 * @brief  XL Scale Factor & Non-Orthogonality Corrections
 */
#define XL_S11					0.989193f
#define XL_S12					0.000006f
#define XL_S13			    	-0.004648f
#define XL_S21					0.000006f
#define XL_S22					1.009450f
#define XL_S23					0.000445f
#define XL_S31			    	-0.004648f
#define XL_S32					0.000445f
#define XL_S33					0.993932f

/*
 * @brief  Bias Offsets
 */
#define GY_XBIASOFFSET_MDPS		-280.0f
#define GY_YBIASOFFSET_MDPS		-490.0f
#define GY_ZBIASOFFSET_MDPS		-140.0f
#define XL_XBIASOFFSET_MG		-4.939707f
#define XL_YBIASOFFSET_MG		-20.213588f
#define XL_ZBIASOFFSET_MG		1.963071f

/*
 * @brief  IMU Status Type Alias
 */
#define LSM6DSOX_OK				IMU_OK
#define LSM6DSOX_ERROR_WARN		IMU_ERROR_WARN
#define LSM6DSOX_ERROR_FATAL	IMU_ERROR_FATAL

typedef imu_status_t lsm6dsox_interface_status_t;

/*
 * @brief  STM Device Context Handle
 */
static stmdev_ctx_t dev_ctx;


/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    pointer to sensor bus handler
 *
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 * @retval 0
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
	if (handle == phi2c) {
		HAL_I2C_Mem_Write(handle, LSM6DSOX_I2C_ADD_L, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);

	} /* else if (handle == phspi) {
	  HAL_SPI_Transmit();

	} */

	return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    pointer to sensor bus handler
 *
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 * @retval 0
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
	if (handle == phi2c) {
		HAL_I2C_Mem_Read(handle, LSM6DSOX_I2C_ADD_L, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);

	} /* else if (handle == phspi) {
	  HAL_SPI_Receive_DMA();

	} */

	return 0;
}

/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 * @retval None
 */
static void platform_delay(uint32_t ms) {
	HAL_Delay(ms);
}

/**
  * @brief lsm6dsox configuration & setup
  *
  * @retval lsm6dsox status type
  */
static lsm6dsox_interface_status_t lsm6dsox_init(void) {
	uint8_t whoamI, rst;

	/* Initialize mems driver interface */
	dev_ctx.write_reg = platform_write;
	dev_ctx.read_reg = platform_read;
	dev_ctx.mdelay = platform_delay;
	dev_ctx.handle = platform_handle;

	/* Check device ID */
	lsm6dsox_device_id_get(&dev_ctx, &whoamI);
	if (whoamI != LSM6DSOX_ID)
	  return LSM6DSOX_ERROR_FATAL;

	/* Restore default configuration */
	lsm6dsox_reset_set(&dev_ctx, PROPERTY_ENABLE);
	do {
	  lsm6dsox_reset_get(&dev_ctx, &rst);
	} while (rst);

	/* Disable I3C interface */
	lsm6dsox_i3c_disable_set(&dev_ctx, LSM6DSOX_I3C_DISABLE);

	/* Enable Block Data Update */
	lsm6dsox_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	// FIFO?

	/* Set Power Mode */
	lsm6dsox_xl_power_mode_set(&dev_ctx, LSM6DSOX_HIGH_PERFORMANCE_MD);
	lsm6dsox_gy_power_mode_set(&dev_ctx, LSM6DSOX_GY_HIGH_PERFORMANCE);

	/* Set Output Data Rate */
	lsm6dsox_xl_data_rate_set(&dev_ctx, LSM6DSOX_XL_ODR_417Hz);
	lsm6dsox_gy_data_rate_set(&dev_ctx, LSM6DSOX_GY_ODR_417Hz);

	/* Set full scale */
	lsm6dsox_xl_full_scale_set(&dev_ctx, LSM6DSOX_2g);
	lsm6dsox_gy_full_scale_set(&dev_ctx, LSM6DSOX_2000dps);

	/* Enable Time Stamp */
	lsm6dsox_timestamp_set(&dev_ctx, PROPERTY_ENABLE);

	/*
	 * Configure filtering chain (No aux interface)
	 *
	 * Accelerometer - LPF1 + LPF2 path
	 */
	lsm6dsox_xl_hp_path_on_out_set(&dev_ctx, LSM6DSOX_LP_ODR_DIV_10);
	lsm6dsox_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);

	/*
	 * uint8_t offset[3] = {};
	 *
	 * Weight of XL user offset
	 * lsm6dsox_xl_offset_weight_set(&dev_ctx, LSM6DSOX_LSb_1mg);
	 *
	 * Accelerometer X,Y,Z axis user offset correction expressed
	 * in two’s complement.
	 *
	 lsm6dsox_xl_usr_offset_x_set(&dev_ctx, &offset[0]);
	 lsm6dsox_xl_usr_offset_y_set(&dev_ctx, &offset[1]);
	 lsm6dsox_xl_usr_offset_z_set(&dev_ctx, &offset[2]);
	 lsm6dsox_xl_usr_offset_set(&dev_ctx, PROPERTY_ENABLE);
	 */

	return LSM6DSOX_OK;
}

/**
  * @brief lsm6dsox deinit
  *
  * @retval lsm6dsox status type
  */
static lsm6dsox_interface_status_t lsm6dsox_deinit(void) {
	/* Restore default configuration */
	lsm6dsox_reset_set(&dev_ctx, PROPERTY_ENABLE);

	/* Reset mems driver interface */
	dev_ctx.write_reg = NULL;
	dev_ctx.read_reg = NULL;
	dev_ctx.mdelay = NULL;
	dev_ctx.handle = NULL;

	return LSM6DSOX_OK;
}

/*
 * @brief	get IMU data in engineering units
 *
 * @param	data	generic sensor handle pointer to store updated measurements
 * @retval	lsm6dsox status type
 */
static lsm6dsox_interface_status_t lsm6dsox_read(void *data) {
	lsm6dsox_reg_t reg;
	int16_t data_raw_acceleration[3];
	int16_t data_raw_angular_rate[3];
	imu_6D_t *imu = (imu_6D_t*) data;

    /* Read output only if new data is available */
	lsm6dsox_status_reg_get(&dev_ctx, &reg.status_reg);

    if (reg.status_reg.xlda || reg.status_reg.gda) {
    	/* Read time stamp value */
    	imu->prev_timestamp = imu->curr_timestamp;
    	lsm6dsox_timestamp_raw_get(&dev_ctx, &(imu->curr_timestamp));
    }

    if (reg.status_reg.xlda) {
    	/* Read acceleration field data */
		memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
		lsm6dsox_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
		imu->accel_x = lsm6dsox_from_fs2_to_mg(data_raw_acceleration[0]);
		imu->accel_y = lsm6dsox_from_fs2_to_mg(data_raw_acceleration[1]);
		imu->accel_z = lsm6dsox_from_fs2_to_mg(data_raw_acceleration[2]);
    }

    if (reg.status_reg.gda) {
		/* Read angular rate field data */
		memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
		lsm6dsox_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate);
		imu->rate_x = lsm6dsox_from_fs2000_to_mdps(data_raw_angular_rate[0]);
		imu->rate_y = lsm6dsox_from_fs2000_to_mdps(data_raw_angular_rate[1]);
		imu->rate_z = lsm6dsox_from_fs2000_to_mdps(data_raw_angular_rate[2]);
    }

    return LSM6DSOX_OK;
}

/*
 * @brief  LSM6DSOX IMU Interface Driver
 */
const imu_interface_t lsm6dsox_driver = {
	.init = lsm6dsox_init,
	.deinit = lsm6dsox_deinit,
	.read = lsm6dsox_read
};
