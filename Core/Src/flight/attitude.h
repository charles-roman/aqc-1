/*
 * attitude.h
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_ATTITUDE_H_
//#define SRC_ATTITUDE_H_
//#endif /* SRC_ATTITUDE_H_ */

#include "system.h"

/* Private defines -----------------------------------------------------------*/

/* SETTINGS-------------------------------------------------------------------*/ //CAUTION WHEN ADJUSTING
//COMPLEMENTARY FILTER GAINS
#define COMP_FILT_GAIN_XL	 	0.02
#define COMP_FILT_GAIN_GYRO	 	(1 - COMP_FILT_GAIN_XL)
/*----------------------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
float get_accel_vec_mag(sensor_3d *accel);

void get_attitude(device *imu, systemState *st);
