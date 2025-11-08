/*
 * system.h
 *
 *  Created on: Oct 17, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "flight/attitude.h"

/* Exported functions prototypes ---------------------------------------------*/
bool ready_to_fly(float accel_z, const attitude_est_t *est, float throttle_req);
