/*
 * timer.h
 *
 *  Created on: Oct 15, 2023
 *      Author: charlieroman
 */

#pragma once

#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
void Cache_TIMCLKRefFreq(TIM_HandleTypeDef *htim);

void start_timer(TIM_HandleTypeDef *htim);

void stop_timer(TIM_HandleTypeDef *htim);

void delay(uint32_t ms);

uint32_t millis(void);

uint32_t Get_TIM2CLKRefFreqHz(void);

uint32_t Get_TIM3CLKRefFreqHz(void);
