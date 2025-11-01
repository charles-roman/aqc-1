/*
 * system.c
 *
 *  Created on: Oct 17, 2023
 *      Author: charlieroman
 */

#include "sensors/imu/imu.h"
#include "flight/system.h"
//#include "position.h"
#include "flight/attitude.h"
#include "flight/pid.h"
#include "flight/mixer.h"
#include "common/maths.h"

/**
  * @brief determine motor commands and update ESC signals
  *
  * @param  sp		pointer to sensorPackage struct
  * @param  st		pointer to systemState struct
  * @param  cmd		pointer to mtrCommands struct
  *
  * @retval None
  */
void actuator_set(sensorPackage *sp, systemState *st, mtrCommands *cmd)
{
	/* Motor Mixing Algorithm */
	motor_mixing(st, cmd);

	/* Thrust Compensation for Roll/Pitch to Maintain Altitude */
	//thrust_compensation(&sp->imu.accel, cmd);
}
