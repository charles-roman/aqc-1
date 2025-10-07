/*
 * setup.h
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_SETUP_H_
//#define SRC_SETUP_H_
//#endif /* SRC_SETUP_H_ */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "../flight/system.h"
#include "../sensors/imu.h"

/* External Variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;

/* Exported functions prototypes ---------------------------------------------*/
void rcInput(uint8_t enable);

void escInput(uint8_t enable);

void arm_drone(uint8_t arm);

uint8_t ready_to_fly(device *imu, systemState *st);

void sensor_setup(sensorPackage *sp);
