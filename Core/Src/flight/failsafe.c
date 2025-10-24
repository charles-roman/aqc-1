/*
 * failsafe.c
 *
 *  Created on: Oct 19, 2023
 *      Author: charlieroman
 */

#include <math.h>
#include "flight/failsafe.h"
#include "flight/system.h"
#include "rx/rx.h"
#include "sensors/imu/imu.h"

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
