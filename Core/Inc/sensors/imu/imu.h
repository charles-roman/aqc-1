/*
 * imu.h
 *
 *  Created on: Oct 14, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "sensors/sensor.h"

/* Exported macros -----------------------------------------------------------*/
#define IMU_OK			SENSOR_OK
#define IMU_ERROR_WARN	SENSOR_ERROR_WARN
#define IMU_ERROR_FATAL	SENSOR_ERROR_FATAL

/* Exported aliases ----------------------------------------------------------*/
typedef sensor_status_t imu_status_t;
typedef sensor_interface_t imu_interface_t;

/* Exported types ------------------------------------------------------------*/
typedef struct imuSixDOF {
	float accel_x;
	float accel_y;
	float accel_z;
	float rate_x;
	float rate_y;
	float rate_z;
	uint32_t dt;
} imu_6D_t;

/* External variables --------------------------------------------------------*/
extern const I2C_HandleTypeDef* phi2c;
// extern const SPI_HandleTypeDef* phspi;
extern const void* platform_handle;

/* Exported functions --------------------------------------------------------*/
imu_status_t imu_init(void);

imu_status_t imu_deinit(void);

imu_status_t imu_read(void *data);
