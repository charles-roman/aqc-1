/*
 * attitude.h
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include "sensors/imu/imu.h"

/* Exported Types ------------------------------------------------------------*/
/**
  * @brief  Attitude / Angular Rates Status Type
  */
typedef enum {
	ATTITUDE_OK,
	ATTITUDE_ERROR_WARN,
	ATTITUDE_ERROR_FATAL
} att_status_t;

/**
  * @brief  Attitude / Angular Rates Estimate Type
  */
typedef struct {
	float roll_angle_deg;
	float pitch_angle_deg;
	// float yaw_angle_deg; (this requires magnetometer readings)
	float roll_rate_dps;
	float pitch_rate_dps;
	float yaw_rate_dps;
} att_estimate_t;

/* Exported functions prototypes ---------------------------------------------*/
att_status_t attitude_update(const imu_6D_t *imu, att_estimate_t *est);

bool attitude_is_right_side_up(const imu_6D_t *imu);

bool attitude_within_limits(const att_estimate_t *est);
