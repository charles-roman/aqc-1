/*
 * attitude.c
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#include <math.h>
#include "flight/attitude.h"
#include "flight/system.h"
#include "common/maths.h"
#include "sensors/imu/imu.h"


/**
  * @brief gets acceleration vector magnitude from accelerometer
  *
  * @param accel	pointer to sensor_3d struct
  * @retval mag		value of magnitude
  */
float get_accel_vec_mag(sensor_3d *accel)
{
	float accel_x = accel->x;
	float accel_y = accel->y;
	float accel_z = accel->z;

	float mag = sqrt(sq(accel_x) + sq(accel_y) + sq(accel_z));

	return mag;
}

/**
  * @brief gets attitude of vehicle in terms of Euler angles w.r.t body frame
  *
  * @param imu		pointer to (imu) device struct
  * @param st		pointer to systemState struct
  *
  * @retval None
  */
void get_attitude(device *imu, systemState *st)
{
	float accel_x = imu->accel.x;
	float accel_y = imu->accel.y;
	float accel_z = imu->accel.z;
	float gyro_x = imu->gyro.x;
	float gyro_y = imu->gyro.y;
	float gyro_z = imu->gyro.z;
	float dt = imu->timestep;

	/* Convert accel data to roll & pitch angle estimates */
	float roll_est_xl = RAD_TO_DEG(atan2(accel_y, sqrt(sq(accel_z) + sq(accel_x))));
	float pitch_est_xl = RAD_TO_DEG(atan2(-accel_x, sqrt(sq(accel_z) + sq(accel_y))));

	/* Convert gyro data to roll & pitch angle estimates */
	float roll_est_gyro = (MDPS_TO_DPS(gyro_x)*dt) + st->roll.estimate;
	float pitch_est_gyro = (MDPS_TO_DPS(gyro_y)*dt) + st->pitch.estimate;

	/* Scale and combine estimates and update buffers */
	st->roll.estimate = (COMP_FILT_GAIN_GYRO)*roll_est_gyro + (COMP_FILT_GAIN_XL)*roll_est_xl;
	st->pitch.estimate = (COMP_FILT_GAIN_GYRO)*pitch_est_gyro + (COMP_FILT_GAIN_XL)*pitch_est_xl;

	/* Read yawrate and update buffer */
	st->yaw.estimate = (MDPS_TO_DPS(gyro_z)); //yawrate
}
