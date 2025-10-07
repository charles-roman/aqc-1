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
#include "main.h"
#include "system.h"

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;

/* Private defines -----------------------------------------------------------*/

/* SETTINGS-------------------------------------------------------------------*/ // CAUTION WHEN ADJUSTING
#define MTR_CMD_MIN			3000.0f
#define MTR_CMD_MAX			6000.0f
#define MTR_CMD_RANGE       (MTR_CMD_MAX - MTR_CMD_MIN)
#define MTR_CMD(percent)	((MTR_CMD_RANGE * percent/100.0f) + MTR_CMD_MIN) // Max is 100% (this would unlock full motor potential)
#define MTR_CMD_LIFTOFF		3725.0f
#define MTR_CMD_IDLE		MTR_CMD(0)
#define MTR_CMD_LIMIT		MTR_CMD(100)
/*----------------------------------------------------------------------------*/

// 3036/3037 (lowest movement cmd)
// 3055/3056 (lowest continuous spin speed cmd)
// 3650 (liftoff cmd)

/* Exported types ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void motor_mixing(systemState *st, mtrCommands *cmd);

void thrust_compensation(sensor_3d *accel, mtrCommands *cmd);

void duty_cycle_set(mtrCommands *cmd);
