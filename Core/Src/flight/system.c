/*
 * system.c
 *
 *  Created on: Oct 17, 2023
 *      Author: charlieroman
 */

#include "../sensors/imu.h"
#include "../sensors/mag.h"
//#include "../sensors/bar.h"
//#include "../sensors/gps.h"
#include "system.h"
//#include "position.h"
#include "attitude.h"
#include "pid.h"
#include "mixer.h"
#include "../rx/pwm.h"
#include "../common/maths.h"

/**
  * @brief user input and system requests
  *
  * @param  st		pointer to systemState struct
  * @retval None
  */
void get_state_request(systemState *st)
{
	get_user_input(st);
	//update_position_request();
}

/**
  * @brief read sensor data for all devices
  *
  * @param  sp		pointer to sensorPackage struct
  * @retval None
  */
void read_sensor_data(sensorPackage *sp)
{
	read_imu_data(&sp->imu);
	//read_mag_data(&sp->mag);
	//lsm6dsox_read_data(&sp->imu);
	//lis3mdl_read_data(&sp->mag);
	//mtk3333_read_data(&sp->gps);
	//bmp390_read_data(&sp->bar);
}

/**
  * @brief determine system state (attitude, position, etc.)
  *
  * @param  sp		pointer to sensorPackage struct
  * @param  st		pointer to systemState struct
  *
  * @retval None
  */
void estimate_state(sensorPackage *sp, systemState *st)
{
	get_attitude(&sp->imu, st);
	//get_position(sp->gps, sp->bar, est->position);
}

/**
  * @brief control system state (attitude, position, throttle, etc.)
  *
  * @param  sp		pointer to sensorPackage struct
  * @param  st		pointer to systemState struct
  *
  * @retval None
  */
void control_state(sensorPackage *sp, systemState *st)
{

	/* Map Throttle Request to Command */
	st->throttle.command = mapf(st->throttle.request, THROTTLE_MIN, THROTTLE_MAX, MTR_CMD(0), MTR_CMD(60));

	/* Clamp Integrator Path at Low Throttle for Takeoff */
	if (st->throttle.command < MTR_CMD_LIFTOFF)
	{
		st->roll.clamp = 0;
		st->pitch.clamp = 0;
		st->yaw.clamp = 0;
	}

	/* Attitude Control (Inner Loop Roll & Pitch) */
	pid_control(&st->roll, sp->imu.timestep);
	pid_control(&st->pitch, sp->imu.timestep);
	pid_control(&st->yaw, sp->imu.timestep);

}

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

	/* Set Duty Cycle */
	duty_cycle_set(cmd);
}
