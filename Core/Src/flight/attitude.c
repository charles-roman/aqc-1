/*
 * attitude.c
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "flight/attitude.h"
#include "flight/rc_input.h"
#include "common/maths.h"
#include "common/settings.h"

/**
  * @brief  Attitude Estimation Filter Selection Setting
  */
#define ATTITUDE_FILT			CONFIG_ATTITUDE_FILT

/**
  * @brief  Complementary Filter Settings
  */
#define COMP_FILT_GAIN_XL	 	CONFIG_COMP_FILT_GAIN_XL
#define COMP_FILT_GAIN_GYRO	 	CONFIG_COMP_FILT_GAIN_GYRO

/**
  * @brief  Attitude Angle Take-off Limit Settings
  */
#define ROLL_TAKEOFF_LIMIT_DEG	CONFIG_ROLL_TAKEOFF_LIMIT_DEG
#define PITCH_TAKEOFF_LIMIT_DEG	CONFIG_PITCH_TAKEOFF_LIMIT_DEG

/**
  * @brief gets attitude of quad-copter in terms of Euler angles w.r.t body frame via complementary filter
  *
  * @param  imu		read-only pointer to imu 6d sensor handle
  * @param	est		pointer to attitude handle
  *
  * @retval None
  */
static void complementary_filter(const imu_6D_t *imu, att_estimate_t *est) {
	/*
	 * No Inf check needed due to bounded xl data
	 * No NaN check needed due to guaranteed valid xl data
	 */

	/* Compute Timestep */
	float dt = (float) (imu->curr_timestamp - imu->prev_timestamp);

	/* Convert accel data to roll and pitch angle estimates */
	float xl_roll_est_deg = RAD_TO_DEG(atan2f(imu->accel_y, sqrtf(sq(imu->accel_z) + sq(imu->accel_x))));
	float xl_pitch_est_deg = RAD_TO_DEG(atan2f(-(imu->accel_x), sqrtf(sq(imu->accel_z) + sq(imu->accel_y))));

	/* Convert gyro data to roll and pitch angle estimates */
	float gyro_roll_est_deg = (est->roll_rate_dps * USEC_TO_SEC(dt)) + est->roll_angle_deg;
	float gyro_pitch_est_deg = (est->pitch_rate_dps * USEC_TO_SEC(dt)) + est->pitch_angle_deg;

	/* Apply complementary filter to get combined estimates */
	est->roll_angle_deg = (COMP_FILT_GAIN_GYRO) * gyro_roll_est_deg + (COMP_FILT_GAIN_XL) * xl_roll_est_deg;
	est->pitch_angle_deg = (COMP_FILT_GAIN_GYRO) * gyro_pitch_est_deg + (COMP_FILT_GAIN_XL) * xl_pitch_est_deg;
}

/**
  * @brief updates angular rates and (conditionally) attitude of quad-copter through configured filter
  *
  * @param  imu		read-only pointer to imu 6d sensor handle
  * @param	est		pointer to attitude handle
  *
  * @retval attitude status type
  */
att_status_t attitude_update(const imu_6D_t *imu, att_estimate_t *est) {
	/* Get Rates */
	est->roll_rate_dps = (MDPS_TO_DPS(imu->rate_x));	// roll rate gets gyro x-axis rate
	est->pitch_rate_dps = (MDPS_TO_DPS(imu->rate_y));	// pitch rate gets gyro y-axis rate
	est->yaw_rate_dps = (MDPS_TO_DPS(imu->rate_z));		// yaw rate gets gyro z-axis rate */

	/* Get Angles (if in Angle Mode) */
	mode_status_t flight_mode = rc_get_flight_mode();
	if (flight_mode == ANGLE_MODE) {
		#if ATTITUDE_FILT == COMP_FILT_ID
			complementary_filter(imu, est);
		#else
			#error "Invalid Attitude Filter Configuration"
		#endif
	}

	return ATTITUDE_OK;
}

/**
  * @brief determines if quad-copter is right side up
  *
  * @param  imu		read-only pointer to imu 6d sensor handle
  * @retval			boolean
  */
bool attitude_is_right_side_up(const imu_6D_t *imu) {
	/* Check sign of accel vector z component */
	return SIGN(imu->accel_z) == POSITIVE;
}

/**
  * @brief determines if quad-copter attitude is within limits
  *
  * @param  est		read-only pointer to attitude handle
  * @retval			boolean
  */
bool attitude_within_limits(const att_estimate_t *est) {
	/* Check roll & pitch angles are within tolerance */
	return (fabs(est->roll_angle_deg) < ROLL_TAKEOFF_LIMIT_DEG) && (fabs(est->pitch_angle_deg) < PITCH_TAKEOFF_LIMIT_DEG);
}
