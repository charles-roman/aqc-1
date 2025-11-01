/*
 * system.h
 *
 *  Created on: Oct 17, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_SYSTEM_H_
//#define SRC_SYSTEM_H_
//#endif /* SRC_SYSTEM_H_ */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "sensors/imu/imu.h"

/* Exported types ------------------------------------------------------------*/
typedef struct sensorPackage {
	device imu;
	sensor_3d mag;
//	sensor_1d bar;
//	sensor_3d gps;
} sensorPackage;

typedef struct state {
	float request;
	float estimate;
	float command;
	float error;
	float command_limit;
	float gains[3];
	float prev_estimate;
	float integrator;
	float differentiator;
	uint8_t clamp;
} state;

typedef struct systemState {
	state roll;
	state pitch;
	state yaw;
	state xpos;
	state ypos;
	state zpos;
	state throttle;
} systemState;

/* Exported functions prototypes ---------------------------------------------*/
void actuator_set(sensorPackage *sp, systemState *st, mtrCommands *cmd);

//void record_data(systemState *st, mtrCommands *cmd);
