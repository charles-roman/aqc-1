/*
 * pid.h
 *
 *  Created on: Oct 16, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_PID_H_
//#define SRC_PID_H_
//#endif /* SRC_PID_H_ */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "system.h"
#include "../common/maths.h"

/* Private defines -----------------------------------------------------------*/
/* SETTINGS-------------------------------------------------------------------*/ // CAUTION WHEN ADJUSTING, CRITICAL SETTINGS
#define ROLL_PID			{8.5, 0, 0}//{8.5, 8.0, 0.80}
#define PITCH_PID			{11.5, 11.0, 0.85}
#define YAW_PID				{7.5, 5.5, 0}
#define XPOS_PID			{1, 1, 1}
#define YPOS_PID			{1, 1, 1}
#define ZPOS_PID			{1, 1, 1}

#define ROLL_CMD_LIM		150
#define PITCH_CMD_LIM		150
#define YAW_CMD_LIM			150
#define XPOS_CMD_LIM		0
#define YPOS_CMD_LIM		0
#define ZPOS_CMD_LIM		0

#define LPF_CUTOFF_FREQ		2*PI*75.0f // rad/s
/*----------------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef struct {
	float error;
	float error_total;
	float gains[3];
	uint8_t clamp;
} pidProfile;

/* Exported functions prototypes ---------------------------------------------*/
void pid_control(state *st, float dt);
