/*
 * hardware.h
 *
 *  Created on: Oct 16, 2025
 *      Author: charlieroman
 */

#pragma once

#include "stm32f4xx_hal.h"

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim8;

/* Exported types ------------------------------------------------------------*/
typedef enum {
	UNCONFIGURED = 0U,
	CONFIGURED   = !UNCONFIGURED
} hw_config_t;

/* Exported macros -----------------------------------------------------------*/
#define HTIM2			CONFIGURED
#define HTIM3			CONFIGURED
#define HTIM4			CONFIGURED
#define HTIM6			CONFIGURED
#define HTIM8			CONFIGURED

