/*
 * attitude.h
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "sensors/imu/imu.h"
#include "flight/rc_input.h"
#include "flight/pid.h"

/* Exported Types ------------------------------------------------------------*/
/**
  * @brief  Attitude / Angular Rates Status Type
  */
typedef enum {
	ATTITUDE_OK,
	ATTITUDE_ERROR_WARN,
	ATTITUDE_ERROR_FATAL
} attitude_status_t;

/**
  * @brief  Attitude / Angular Rates Estimate Type
  */
typedef struct {
	float roll_angle_deg;
	float pitch_angle_deg;
	float roll_rate_dps;
	float pitch_rate_dps;
	float yaw_rate_dps;
} attitude_est_t;

/**
  * @brief  Attitude / Angular Rates Command Type
  */
typedef struct {
	float roll;
	float pitch;
	float yaw;
} attitude_cmd_t;

/* Exported functions prototypes ---------------------------------------------*/
attitude_status_t attitude_estimator_update(const imu_6D_t *imu, attitude_est_t *est);

attitude_status_t attitude_controller_update(attitude_cmd_t *cmd, const rc_reqs_t *req, const attitude_est_t *est, float dt);

void attitude_controller_init(void);

bool attitude_is_right_side_up(float accel_z);

bool attitude_within_limits(const attitude_est_t *est);
