/*
 * pwm.h
 *
 *  Created on: Oct 13, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_PWM_H_
//#define SRC_PWM_H_
//#endif /* SRC_PWM_H_ */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "main.h"
#include "../flight/system.h"

/* Private defines -----------------------------------------------------------*/

/* SETTINGS-------------------------------------------------------------------*/ //CAUTION WHEN ADJUSTING
//USER INPUT STATE LIMITS
#define ROLL_MIN		-10.0f		//_DEG
#define ROLL_MAX	 	 10.0f
#define PITCH_MIN		-10.0f
#define PITCH_MAX	 	 10.0f
#define YAW_MIN			-180.0f
#define YAW_MAX		 	 180.0f
#define THROTTLE_MIN	 0.0f
#define THROTTLE_MAX	 100.0f 	//(%)
//PWM RANGE
#define PWM_RANGE_MIN 	 988
#define PWM_RANGE_MAX 	 2012
#define PWM_PULSE_MIN    950       // minimum PWM pulse width which is considered valid
#define PWM_PULSE_MAX    2050      // maximum PWM pulse width which is considered valid
#define PWM_FREQ_HZ   	 50
/*----------------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef struct channel {
	uint32_t icval1;
	uint32_t icval2;
	uint32_t icdiff;
	const uint32_t id;
	uint8_t is_first_captured;
	volatile float pulsewidth_us;
} channel;

typedef struct icChannels {
	channel ch1;
	channel ch2;
	channel ch3;
	channel ch4;
} icChannels;

/* Exported functions prototypes ---------------------------------------------*/
void get_user_input(systemState *st);

void read_rc_input(TIM_HandleTypeDef *htim);
