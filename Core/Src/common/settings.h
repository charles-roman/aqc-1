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
#define CONFIG_PWM_FREQ_HZ   	 				50		// (NOTE: Update this to use timer calculations? Or Vice Versa? One must inform other. Do we even need it?)

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

/* MOTOR COMMAND CONFIG SETTINGS----------------------------------------------*/
#define CONFIG_MTR_CMD_MIN						3000.0f	// (3000 => 1ms pulse @ 50Hz => 5% duty cycle)
#define CONFIG_MTR_CMD_MAX						6000.0f // (6000 => 2ms pulse @ 50Hz => 10% duty cycle)
#define CONFIG_MTR_CMD_LIFTOFF					3725.0f
#define CONFIG_MTR_CMD_IDLE_PCT					0.0f
#define CONFIG_MTR_CMD_LIMIT_PCT				100.0f

/*
 * On Full Charged Battery
 *
 * 3036/3037 (lowest movement cmd)
 * 3055/3056 (lowest continuous spin speed cmd)
 * 3650 (liftoff cmd)
 *
 */
