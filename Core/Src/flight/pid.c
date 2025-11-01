/*
 * pid.c
 *
 *  Created on: Oct 16, 2023
 *      Author: charlieroman
 */

#include <math.h>
#include "flight/pid.h"
#include "common/maths.h"

/**
  * @brief pid controller update
  *
  * @param  pid				pointer to pid controller handle
  * @param  setpoint		requested state value
  * @param	measurement		estimated state value
  * @param	dt				timestep
  *
  * @retval pid controller output (constrained)
  */
float pid_update(pid_ctrl_t *pid, float setpoint, float measurement, float dt) {
	/* Error Signal */
    float error = setpoint - measurement;

    /* Integrator (with hold detection anti-windup clamp) */
    if (pid->integrator_enable) {
		/* Command Saturation Check */
		bool is_saturated = fabs(pid->out) >= pid->limit;

		/* Integrator Windup Check */
		bool is_winding = SIGN(error) == SIGN(pid->integrator);

		/* Only integrate if not contributing to saturation */
		if (!(is_saturated && is_winding)) {
			pid->integrator += 0.5f * (error + pid->prev_error) * dt;
			pid->integrator = constrainf(pid->integrator, -pid->integrator_limit, pid->integrator_limit);
		}
    }
    	// can optionally decay integrator slowly when disabled

    /* Band-limited Differentiator (on measurement) */
    pid->differentiator = (2.0f * (pid->prev_measurement - measurement)
    				    + (2.0f * pid->tau - dt) * pid->differentiator)
    					/ (2.0f * pid->tau + dt);

    /* Compute PID Output */
    pid->out = pid->Kp * error
             + pid->Ki * pid->integrator
             + pid->Kd * pid->differentiator;

    /* Cache Error and Measurement */
    pid->prev_error = error;
    pid->prev_measurement = measurement;

    /* Constrain Output */
    return constrainf(pid->out, -pid->limit, pid->limit);
}


/**
  * @brief init pid controller
  *
  * @param  ctrl	pointer pid controller handle
  * @param  config	pointer to pid config handle
  *
  * @retval None
  */
void pid_init(pid_ctrl_t *ctrl, const pid_config_t *config) {
	ctrl->Kp = config->Kp;
	ctrl->Ki = config->Ki;
	ctrl->Kd = config->Kd;
	ctrl->limit = config->limit;
	ctrl->integrator_limit = config->integrator_limit;
	ctrl->tau = RAD_PER_SEC_TO_INTERVAL(config->Wc);

	ctrl->prev_error = 0.0f;
	ctrl->prev_measurement = 0.0f;
	ctrl->integrator = 0.0f;
	ctrl->integrator_enable = true;
	ctrl->differentiator = 0.0f;
	ctrl->out = 0.0f;
}

/**
  * @brief resync pid controller to current state
  *
  * @param  ctrl			pointer pid controller handle
  * @param  setpoint		requested state value
  * @param	measurement		estimated state value
  *
  * @retval None
  */
void pid_resync(pid_ctrl_t *pid, float setpoint, float measurement) {
	pid->prev_error = setpoint - measurement;
	pid->prev_measurement = measurement;
	pid->integrator = 0.0f;
	pid->differentiator = 0.0f;
}

/**
  * @brief reset pid controller
  *
  * @param  ctrl	pointer pid controller handle
  * @retval None
  */
void pid_reset(pid_ctrl_t *pid) {
	pid->prev_error = 0.0f;
	pid->prev_measurement = 0.0f;
	pid->integrator = 0.0f;
	pid->integrator_enable = true;
	pid->differentiator = 0.0f;
	pid->out = 0.0f;
}
