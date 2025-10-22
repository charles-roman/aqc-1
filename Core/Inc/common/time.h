/*
 * timer.h
 *
 *  Created on: Oct 15, 2023
 *      Author: charlieroman
 */

#pragma once

#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
uint32_t Get_TIMxClkRefFreqHz(TIM_HandleTypeDef *htim);

uint32_t Get_TIMxClkRefFreqMHz(TIM_HandleTypeDef *htim);

void start_timer(TIM_HandleTypeDef *htim);

void stop_timer(TIM_HandleTypeDef *htim);

void delay(uint32_t ms);

uint32_t millis(void);
