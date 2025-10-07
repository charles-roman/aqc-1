/*
 * timer.h
 *
 *  Created on: Oct 15, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_TIMER_H_
//#define SRC_TIMER_H_
//#endif /* SRC_TIMER_H_ */

#include "main.h"

/* Private defines -----------------------------------------------------------*/
#define APB1_CLK_FREQ_MHZ	84

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim6;

/* Exported types ------------------------------------------------------------*/
typedef struct timer {
	uint8_t first_iteration;
	uint16_t t1;
	uint16_t t2;
	uint16_t dt;
	uint32_t timestamp;
} timer;

/* Exported functions prototypes ---------------------------------------------*/
void delay(uint32_t ms);

uint32_t get_timestamp(void);

void start_timer(TIM_HandleTypeDef *htim);

void stop_timer(TIM_HandleTypeDef *htim);

uint32_t millis(void); //__weak
