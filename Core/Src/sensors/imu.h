/*
 * imu.h
 *
 *  Created on: Oct 14, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_IMU_H_
//#define SRC_IMU_H_
//#endif /* SRC_IMU_H_ */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "main.h"
#include "../common/maths.h"

/* Private defines -----------------------------------------------------------*/

/* SETTINGS-------------------------------------------------------------------*/	//CAUTION WHEN ADJUSTING
//XL SCALE FACTOR & NONORTHOGONALITY CORRECTIONS
#define S11_xl					0.989193f
#define S12_xl					0.000006f
#define S13_xl				    -0.004648f
#define S21_xl					0.000006f
#define S22_xl					1.009450f
#define S23_xl					0.000445f
#define S31_xl				    -0.004648f
#define S32_xl					0.000445f
#define S33_xl					0.993932f
//BIAS OFFSETS
#define GYRO_XBIASOFFSET_MDPS	-280.0f
#define GYRO_YBIASOFFSET_MDPS	-490.0f
#define GYRO_ZBIASOFFSET_MDPS	-140.0f
#define ACCEL_XBIASOFFSET_MG	-4.939707f
#define ACCEL_YBIASOFFSET_MG	-20.213588f
#define ACCEL_ZBIASOFFSET_MG	1.963071f
//CUTOFF FREQ
#define GYRO_LPF_CUTOFF_FREQ	2*PI*500.0f //(rad/s)
/*----------------------------------------------------------------------------*/

extern I2C_HandleTypeDef hi2c1;

/* Exported types ------------------------------------------------------------*/
typedef union {
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union {
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

typedef struct threedofSensor {
	float x;
	float y;
	float z;
	float x_filtered;
	float y_filtered;
	float z_filtered;
	float timestep;
	axis3bit16_t raw;
} sensor_3d;

typedef struct imuDevice {
	sensor_3d accel;
	sensor_3d gyro;
	float timestep;
} device;

/* Exported functions prototypes ---------------------------------------------*/

void lsm6dsox_setup(device *imu);

void read_imu_data(device *imu);

