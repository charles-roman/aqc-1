/*
 * mixer.c
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#include <math.h>
#include "main.h"
#include "mixer.h"
#include "attitude.h"
#include "system.h"
#include "../common/maths.h"

/**
  * @brief motor mixing algorithm
  *
  * @param  st		pointer to systemState struct
  * @param  cmd		pointer to mtrCommands struct
  *
  * @retval None
  */
void motor_mixing(systemState *st, mtrCommands *cmd)
{
	float throttle_cmd, roll_cmd, pitch_cmd, yaw_cmd;
	yaw_cmd = st->yaw.command;
	roll_cmd = st->roll.command;
	pitch_cmd = st->pitch.command;
	throttle_cmd = st->throttle.command;

	cmd->mtr1 = throttle_cmd;// + roll_cmd;// - pitch_cmd + yaw_cmd;
	cmd->mtr2 = throttle_cmd;// + roll_cmd;// + pitch_cmd - yaw_cmd;
	cmd->mtr3 = throttle_cmd;// - roll_cmd;// - pitch_cmd - yaw_cmd;
	cmd->mtr4 = throttle_cmd;// - roll_cmd;// + pitch_cmd + yaw_cmd;
}

/**
  * @brief thrust compensation algorithm
  *
  * @param  st		pointer to systemState struct
  * @param  cmd		pointer to mtrCommands struct
  *
  * @retval None
  */
void thrust_compensation(sensor_3d *accel, mtrCommands *cmd)
{
	float thrust_ratio = sqrt(1/accel->z);

	if isnan(thrust_ratio)
		thrust_ratio = 1;

	cmd->mtr1 = thrust_ratio*(cmd->mtr1) - MTR_CMD_RANGE*(thrust_ratio - 1);
	cmd->mtr2 = thrust_ratio*(cmd->mtr2) - MTR_CMD_RANGE*(thrust_ratio - 1);
	cmd->mtr3 = thrust_ratio*(cmd->mtr3) - MTR_CMD_RANGE*(thrust_ratio - 1);
	cmd->mtr4 = thrust_ratio*(cmd->mtr4) - MTR_CMD_RANGE*(thrust_ratio - 1);
}

/**
  * @brief set duty cycle for pwm signals to ESCs
  *
  * @param  cmd		pointer to mtrCommands struct
  * @retval None
  */
void duty_cycle_set(mtrCommands *cmd)
{
	/* NaN Check */
	if isnan(cmd->mtr1)
		cmd->mtr1 = MTR_CMD_MIN;
	if isnan(cmd->mtr2)
		cmd->mtr2 = MTR_CMD_MIN;
	if isnan(cmd->mtr3)
		cmd->mtr3 = MTR_CMD_MIN;
	if isnan(cmd->mtr4)
		cmd->mtr4 = MTR_CMD_MIN;

	/* Constrain */
	cmd->mtr1 = constrainf(cmd->mtr1, MTR_CMD_IDLE, MTR_CMD_LIMIT);
	cmd->mtr2 = constrainf(cmd->mtr2, MTR_CMD_IDLE, MTR_CMD_LIMIT);
	cmd->mtr3 = constrainf(cmd->mtr3, MTR_CMD_IDLE, MTR_CMD_LIMIT);
	cmd->mtr4 = constrainf(cmd->mtr4, MTR_CMD_IDLE, MTR_CMD_LIMIT);

	/* Set Duty Cycle; (can adjust CCR directly for speed) */
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, cmd->mtr3);
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, cmd->mtr4);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, cmd->mtr2);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, cmd->mtr1);
}
