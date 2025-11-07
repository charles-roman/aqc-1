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
extern TIM_HandleTypeDef htim8;

extern I2C_HandleTypeDef hi2c1;

/* Exported types ------------------------------------------------------------*/
typedef enum {
	UNCONFIGURED = 0U,
	CONFIGURED   = !UNCONFIGURED
} hw_config_t;

/* Exported macros -----------------------------------------------------------*/
#define HTIM2				CONFIGURED
#define HTIM3				CONFIGURED
#define HTIM4				CONFIGURED
#define HTIM8				CONFIGURED

#define HI2C1				CONFIGURED

#define LED_Pin 			GPIO_PIN_1
#define LED_GPIO_Port 		GPIOC

#define ARM_Pin 			GPIO_PIN_2
#define ARM_GPIO_Port 		GPIOC

#define MODE_Pin 			GPIO_PIN_3
#define MODE_GPIO_Port 		GPIOC

#define SD_DETECT_Pin 		GPIO_PIN_12
#define SD_DETECT_GPIO_Port GPIOB
