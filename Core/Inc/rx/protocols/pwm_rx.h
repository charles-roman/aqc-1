/*
 * pwm.h
 *
 *  Created on: Oct 13, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "flight/system.h"
#include "stm32f4xx_hal.h"
#include "common/settings.h"

/* Private defines -----------------------------------------------------------*/
// STATE LIMITS (ANGLE MODE)
#define ROLL_MIN_DEG		CONFIG_ROLL_MIN_DEG
#define ROLL_MAX_DEG	 	CONFIG_ROLL_MAX_DEG
#define PITCH_MIN_DEG		CONFIG_PITCH_MIN_DEG
#define PITCH_MAX_DEG	 	CONFIG_PITCH_MAX_DEG
#define YAW_MIN_DPS			CONFIG_YAW_MIN_DPS
#define YAW_MAX_DPS	 	 	CONFIG_YAW_MAX_DPS
// THROTTLE LIMITS
#define THROTTLE_MIN_PCT	CONFIG_THROTTLE_MIN_PCT
#define THROTTLE_MAX_PCT	CONFIG_THROTTLE_MAX_PCT
// PWM RANGE
#define PWM_PULSE_MIN_US    CONFIG_PWM_PULSE_MIN_US
#define PWM_PULSE_MAX_US    CONFIG_PWM_PULSE_MAX_US
#define PWM_PULSE_MED_US	(PWM_PULSE_MIN_US + PWM_PULSE_MAX_US) / 2
#define PWM_FREQ_HZ   	 	CONFIG_PWM_FREQ_HZ
// CHANNEL ALIASES
#define PITCH_CHANNEL		TIM_CHANNEL_1
#define ROLL_CHANNEL		TIM_CHANNEL_2
#define THROTTLE_CHANNEL	TIM_CHANNEL_3
#define YAW_CHANNEL			TIM_CHANNEL_4

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

/* Exported types ------------------------------------------------------------*/
typedef struct pulse {
	uint8_t id;
	bool is_rising;
	bool is_updated;
	uint16_t ic_val_r;
	uint16_t ic_val_f;
} pulse_t;

typedef enum {
	PITCH_PULSE_ID		= 0x01U,
	ROLL_PULSE_ID		= 0x02U,
	THROTTLE_PULSE_ID	= 0x03U,
	YAW_PULSE_ID		= 0x04U
} pulse_id_t;

/* Exported functions prototypes ---------------------------------------------*/
void get_rc_requests_over_pwm(systemState *st);

void init_ic_timclk_ref_props(void);

void init_pwm_pulse_handles(void);

void start_pwm_input_capture(void);

void stop_pwm_input_capture(void);
