/*
 * failsafe.c
 *
 *  Created on: Oct 19, 2023
 *      Author: charlieroman
 */

#include <math.h>
#include "failsafe.h"
#include "system.h"
#include "../rx/pwm.h"
#include "../sensors/imu.h"

/**
  * @brief determines if throttle is in idle position (or close to it)
  *
  * @param st		pointer to systemState struct
  * @retval ret		logical value (1 or 0)
  */
uint8_t throttle_idle(systemState *st)
{
	uint8_t ret;
	get_user_input(st);

	/* Determine if throttle is within idle tolerance */
	ret = (st->throttle.request < 5);

	return ret;
}

/**
  * @brief determines if quad is currently armed
  *
  * @param None
  * @retval ret		logical value (1 or 0)
  */
uint8_t armed(void)
{
	uint8_t ret;

	/* Determine if arm switch (SE) is active */
	ret = HAL_GPIO_ReadPin(ARM_GPIO_Port, ARM_Pin);

	return ret;
}

/**
  * @brief determines if quadcopter is right side up
  *
  * @param imu		pointer to (imu) device struct
  * @retval ret		logical value (1 or 0)
  */
uint8_t right_side_up(device *imu)
{
	uint8_t ret;

	/* Check sign of accel vector z component */
	ret = (imu->accel.z > 0);

	return ret;
}

/**
  * @brief determines if quadcopter attitude is within limits
  *
  * @param st		pointer to systemState struct
  * @retval ret		logical value (1 or 0)
  */
uint8_t attitude_within_boundaries(systemState *st)
{
	uint8_t ret;

	/* Check roll & pitch angles are within tolerance */
	ret = (fabs(st->roll.estimate) < ROLL_MAX) && (fabs(st->pitch.estimate) < PITCH_MAX);

	return ret;
}

//void fail_safe_mode(systemState *st)
