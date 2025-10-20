/*
 * settings.h
 *
 *  Created on: Oct 7, 2025
 *      Author: charlieroman
 */

#pragma once

#include "maths.h"

// CAUTION WHEN ADJUSTING!!!

/* PWM PROTOCOL CONFIG SETTINGS-----------------------------------------------*/
#define CONFIG_PWM_PULSE_MIN_US 				988.0f
#define CONFIG_PWM_PULSE_MAX_US 				2012.0f
#define CONFIG_PWM_PULSE_PROTO_MIN_US			1000.0f
#define CONFIG_PWM_PULSE_PROTO_MAX_US			2000.0f
#define CONFIG_PWM_PULSE_VALID_MIN_US			950.0f
#define CONFIG_PWM_PULSE_VALID_MAX_US			2050.0f

/* CHANNEL CONFIG SETTINGS ---------------------------------------------------*/
#define CONFIG_ROLL_MIN_DEG						-10.0f
#define CONFIG_ROLL_MAX_DEG	 	 				10.0f
#define CONFIG_PITCH_MIN_DEG					-10.0f
#define CONFIG_PITCH_MAX_DEG	 				10.0f
#define CONFIG_YAW_MIN_DPS						-180.0f
#define CONFIG_YAW_MAX_DPS						180.0f
#define CONFIG_THROTTLE_MIN_PCT	 				0.0f
#define CONFIG_THROTTLE_MAX_PCT	 				100.0f

/* IMU FILTER CONFIG SETTINGS-------------------------------------------------*/
#define CONFIG_GYRO_LPF_CUTOFF_FREQ_RPS 		2 * PI * 500.0f

/* COMPLEMENTARY FILTER CONFIG SETTINGS---------------------------------------*/
#define CONFIG_COMP_FILT_GAIN_XL	 			0.02f
#define CONFIG_COMP_FILT_GAIN_GYRO	 			(1 - COMP_FILT_GAIN_XL)

/* PID CONTROLLER CONFIG SETTINGS---------------------------------------------*/
#define CONFIG_ROLL_PID_GAINS					{8.5f, 8.0f, 0.80f}
#define CONFIG_PITCH_PID_GAINS					{11.5f, 11.0f, 0.85f}
#define CONFIG_YAW_PID_GAINS					{7.5f, 5.5f, 0.0f}
#define CONFIG_ROLL_CMD_LIM						150.0f
#define CONFIG_PITCH_CMD_LIM					150.0f
#define CONFIG_YAW_CMD_LIM						150.0f
#define CONFIG_DERIVATIVE_LPF_CUTOFF_FREQ_RPS	2 * PI * 75.0f	// (NOTE: Might want to break this out into individual cutoff frequencies for separate controllers)

/* SENSOR CONFIG SETTINGS----------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// IMU------------------------------------------------------------------------
#define CONFIG_GYRO_LPF_CUTOFF_FREQ_RPS 		2 * PI * 500.0f

/* PROTOCOL CONFIG SETTINGS--------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// PWM------------------------------------------------------------------------
#define CONFIG_PWM_PULSE_MIN_US 				988.0f
#define CONFIG_PWM_PULSE_MAX_US 				2012.0f
#define CONFIG_PWM_PULSE_PROTO_MIN_US			1000.0f
#define CONFIG_PWM_PULSE_PROTO_MAX_US			2000.0f
#define CONFIG_PWM_PULSE_VALID_MIN_US			950.0f
#define CONFIG_PWM_PULSE_VALID_MAX_US			2050.0f

/* RX CONFIG SETTINGS--------------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// PROTOCOL-------------------------------------------------------------------
#define RX_PWM_PROTOCOL_ID						0U
#define CONFIG_RX_PROTOCOL						RX_PWM_PROTOCOL_ID

/* ESC CONFIG SETTINGS-------------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// PROTOCOL-------------------------------------------------------------------
#define ESC_PWM_PROTOCOL_ID						0U
#define CONFIG_ESC_PROTOCOL						ESC_PWM_PROTOCOL_ID
// MOTOR COMMANDS-------------------------------------------------------------
#define CONFIG_MTR_CMD_IDLE_PCT					18.0f
#define CONFIG_MTR_CMD_LIFTOFF_PCT				24.0f
#define CONFIG_MTR_CMD_LIMIT_PCT				100.0f // Max is 100% (this would unlock full motor potential)

/*
 * On Full Charged Battery
 *
 * 3036/3037 (lowest movement cmd)
 * 3055/3056 (lowest continuous spin speed cmd)
 * 3650 (liftoff cmd)
 *
 */
