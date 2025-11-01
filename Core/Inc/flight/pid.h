/*
 * pid.h
 *
 *  Created on: Oct 16, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
typedef struct {
	float Kp;
	float Ki;
	float Kd;
	float Wc;
	float limit;
	float integrator_limit;
} pid_config_t;

typedef struct {
	float Kp;
	float Ki;
	float Kd;
	float tau;
	float limit;
	// float dt;
	bool integrator_enable;
	float integrator_limit;
	float integrator;
	float differentiator;
	float prev_error;
	float prev_measurement;
	float out;
} pid_ctrl_t;

/* Exported functions prototypes ---------------------------------------------*/
float pid_update(pid_ctrl_t *pid, float measurement, float setpoint, float dt);

void pid_init(pid_ctrl_t *ctrl, const pid_config_t *config);

void pid_resync(pid_ctrl_t *pid, float measurement, float setpoint);

void pid_reset(pid_ctrl_t *pid);
