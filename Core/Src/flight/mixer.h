/*
 * mixer.h
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_MIXER_H_
//#define SRC_MIXER_H_
//#endif /* SRC_MIXER_H_ */

/* Includes ------------------------------------------------------------------*/
#include "system.h"
#include "../esc/esc.h"

/* Exported functions prototypes ---------------------------------------------*/
void motor_mixing(systemState *st, mtr_cmds_t *cmd);

void thrust_compensation(sensor_3d *accel, mtrCommands *cmd);
