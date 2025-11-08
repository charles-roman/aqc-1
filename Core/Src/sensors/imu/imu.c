/*
 * imu.c
 *
 *  Created on: Oct 14, 2023
 *      Author: charlieroman
 */

#include "sensors/imu/imu.h"
#include "sensors/imu/devices/lsm6dsox.h"
#include "common/hardware.h"
#include "common/settings.h"

/*
 * @brief  IMU Device Config Setting(s)
 */
#define IMU_DEVICE				CONFIG_IMU_DEVICE

/*
 * @brief  LPF Config Settings
 */
#define GY_LPF					CONFIG_GY_LPF
#define XL_LPF					CONFIG_XL_LPF
#define GY_LPF_CUTOFF_FREQ_HZ	CONFIG_GY_LPF_CUTOFF_FREQ_HZ	// digital filter
#define XL_LPF_CUTOFF_FREQ_HZ	CONFIG_XL_LPF_CUTOFF_FREQ_HZ	// digital filter

/*
 * @brief  IMU Comm Protocol Config Settings
 */
#define IMU_COMM_PROTOCOL		CONFIG_IMU_COMM_PROTOCOL

/*
 * @brief  IMU Comm Peripheral(s)
 */
#define IMU_I2C_PERIPHERAL		I2C1
// #define IMU_SPI_PERIPHERAL		SPI2

/**
  * @brief  IMU Comm Peripheral Handle Pointers
  */
const I2C_HandleTypeDef* phi2c = NULL;
//const SPI_HandleTypeDef* phspi = NULL;

const void* platform_handle = NULL;

/**
  * @brief  imu driver pointer for imu device interface
  */
static const imu_interface_t *imu_driver = NULL;


/**
  * @brief helper function to get the appropriate comm handle based on hardware config
  *
  * @param  i2c		pointer to i2c type handle
  * @retval pointer to i2c handle type (NULL otherwise)
  */
static I2C_HandleTypeDef* Get_IMU_I2C_Handle(I2C_TypeDef* i2c) {
	#if HI2C1 == CONFIGURED
	if (i2c == I2C1)
		return &hi2c1;
	#endif

	// add more as needed

	return NULL;
}

/*
  * @brief helper function to get the appropriate comm handle based on hardware config
  *
  * @param  spi		pointer to spi type handle
  * @retval pointer to spi handle type (NULL otherwise)
  *
static SPI_HandleTypeDef* Get_IMU_SPI_Handle(SPI_TypeDef* spi) {
	#if HSPI2 == CONFIGURED
	if (spi == SPI2)
		return &hspi2;
	#endif

	// add more as needed

	return NULL;
}
 *
 */

/*
 * @brief imu API call to init imu interface (protocol + device)
 *
 * @retval imu status type
 */
imu_status_t imu_init(void) {
	#if IMU_COMM_PROTOCOL == IMU_I2C_PROTOCOL_ID
		phi2c = Get_IMU_I2C_Handle(IMU_I2C_PERIPHERAL);
		platform_handle = phi2c;
	#else
		#error "Invalid IMU Communication Protocol Configuration"
	#endif

	#if IMU_DEVICE == LSM6DSOX_DEVICE_ID
		imu_driver = &lsm6dsox_driver;
	#else
		#error "Invalid IMU Device Configuration"
	#endif

	if (platform_handle == NULL)
		return IMU_ERROR_FATAL;

	if (!valid_sensor_driver(imu_driver))
		return IMU_ERROR_FATAL;

	return imu_driver->init();
}

/*
 * @brief imu API call to deinit imu interface (protocol + device)
 *
 * @retval imu status type
 */
imu_status_t imu_deinit(void) {
	if (!imu_driver)
		return IMU_ERROR_WARN;

	platform_handle = NULL;

	imu_driver->deinit();
	imu_driver = NULL;

	return IMU_OK;
}

/*
 * @brief imu API call to read values from sensor
 *
 * @param  data		generic pointer to sensor handle
 * @retval imu status type
 */
imu_status_t imu_read(void *data) {
	imu_status_t status;

	if (!imu_driver)
		return IMU_ERROR_FATAL;

	status = imu_driver->read(data);

	/*
	 #if IMU_CALIBRATE_DATA == ENABLED
		imu_calibrate(data);
	#endif
	*/

	/*
	#if GY_LPF == ENABLED
		lowpass_filter(data);
	#endif

	#if XL_LPF == ENABLED
		lowpass_filter(data);
	#endif
	*/

	return status;
}
