/*
 * failsafe.h
 *
 *  Created on: Oct 19, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_FAILSAFE_H_
//#define SRC_FAILSAFE_H_
//#endif /* SRC_FAILSAFE_H_ */

#include <stdint.h>
#include "system.h"
#include "sensors/imu/imu.h"

uint8_t right_side_up(device *imu);

uint8_t attitude_within_boundaries(systemState *st);

//void fail_safe_mode(systemState *st);
