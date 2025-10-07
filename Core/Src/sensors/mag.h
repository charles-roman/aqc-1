/*
 * mag.h
 *
 *  Created on: Nov 25, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_MAG_H_
//#define SRC_MAG_H_
//#endif /* SRC_MAG_H_ */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "imu.h"

/* Private defines -----------------------------------------------------------*/

/* SETTINGS-------------------------------------------------------------------*/	//CAUTION WHEN ADJUSTING
//MAG SCALE FACTOR & NONORTHOGONALITY CORRECTIONS
#define S11_mag					1
#define S12_mag					1
#define S13_mag					1
#define S21_mag					1
#define S22_mag					1
#define S23_mag					1
#define S31_mag					1
#define S32_mag					1
#define S33_mag					1
//BIAS OFFSETS
#define MAG_XBIASOFFSET_mG	 	 0.0f
#define MAG_YBIASOFFSET_mG	 	 0.0f
#define MAG_ZBIASOFFSET_mG	 	 0.0f
/*----------------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void lis3mdl_setup(sensor_3d *mag);

void read_mag_data(sensor_3d *mag);
