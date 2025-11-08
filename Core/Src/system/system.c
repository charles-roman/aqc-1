/*
 * system.c
 *
 *  Created on: Oct 17, 2023
 *      Author: charlieroman
 */

#include "system/system.h"

/**
  * @brief flight ready checks
  *
  * @param	accel_z			z-component of imu accel vector
  * @param	est				read-only pointer to attitude estimate handle
  * @param	throttle_req	throttle request
  *
  * @retval boolean
  */
bool ready_to_fly(float accel_z, const attitude_est_t *est, float throttle_req) {
	return (attitude_is_right_side_up(accel_z) &&
			attitude_within_limits(est) &&
			rc_is_throttle_idle(throttle_req));
}
