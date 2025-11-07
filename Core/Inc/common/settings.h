/*
 * settings.h
 *
 *  Created on: Oct 7, 2025
 *      Author: charlieroman
 */

#pragma once

#include "maths.h"

// CAUTION WHEN ADJUSTING!!!

#define	DISABLED	0U
#define ENABLED		!DISABLED

/* FLIGHT CONFIG SETTINGS----------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// RC INPUT-------------------------------------------------------------------
#define CONFIG_ROLL_MAX_DEG	 	 					10.0f
#define CONFIG_ROLL_MIN_DEG							-CONFIG_ROLL_MAX_DEG
#define CONFIG_ROLL_MAX_DPS							180.0f
#define CONFIG_ROLL_MIN_DPS							-CONFIG_ROLL_MAX_DPS
#define CONFIG_PITCH_MAX_DEG	 					10.0f
#define CONFIG_PITCH_MIN_DEG						-CONFIG_PITCH_MAX_DEG
#define CONFIG_PITCH_MAX_DPS						180.0f
#define CONFIG_PITCH_MIN_DPS						-CONFIG_PITCH_MAX_DPS
#define CONFIG_YAW_MAX_DPS							180.0f
#define CONFIG_YAW_MIN_DPS							-CONFIG_YAW_MAX_DPS

#define THROTTLE_MIN_PCT	 						0.0f
#define THROTTLE_MAX_PCT	 						100.0f

#define CONFIG_THROTTLE_IDLE_TOLERANCE_PCT			2.0f

// ATTITUDE-------------------------------------------------------------------
#define COMP_FILT_ID								0U
#define CONFIG_ATTITUDE_FILT						COMP_FILT_ID

#define CONFIG_COMP_FILT_GAIN_XL	 				0.02f
#define CONFIG_COMP_FILT_GAIN_GYRO	 				(1 - CONFIG_COMP_FILT_GAIN_XL)

#define CONFIG_ROLL_TAKEOFF_LIMIT_DEG				10.0f
#define CONFIG_PITCH_TAKEOFF_LIMIT_DEG				10.0f

#define CONFIG_ROLL_ANGLE_P_GAIN					8.5f
#define CONFIG_ROLL_ANGLE_I_GAIN					8.0f
#define CONFIG_ROLL_ANGLE_D_GAIN					0.80f
#define CONFIG_ROLL_ANGLE_D_LPF_CUTOFF_FREQ_HZ		75.0f
#define CONFIG_ROLL_ANGLE_CMD_LIM_DPS				CONFIG_ROLL_MAX_DPS
#define CONFIG_ROLL_ANGLE_I_CMD_LIM_DPS				CONFIG_ROLL_ANGLE_CMD_LIM_DPS * 0.3

#define CONFIG_PITCH_ANGLE_P_GAIN					11.5f
#define CONFIG_PITCH_ANGLE_I_GAIN					11.0f
#define CONFIG_PITCH_ANGLE_D_GAIN					0.85f
#define CONFIG_PITCH_ANGLE_D_LPF_CUTOFF_FREQ_HZ		75.0f
#define CONFIG_PITCH_ANGLE_CMD_LIM_DPS				CONFIG_PITCH_MAX_DPS
#define CONFIG_PITCH_ANGLE_I_CMD_LIM_DPS			CONFIG_PITCH_ANGLE_CMD_LIM_DPS * 0.3

#define CONFIG_ROLL_RATE_P_GAIN						7.5f
#define CONFIG_ROLL_RATE_I_GAIN						5.5f
#define CONFIG_ROLL_RATE_D_GAIN						0.0f
#define CONFIG_ROLL_RATE_D_LPF_CUTOFF_FREQ_HZ		75.0f
#define CONFIG_ROLL_RATE_CMD_LIM_PCT				5.0f
#define CONFIG_ROLL_RATE_I_CMD_LIM_PCT				CONFIG_ROLL_RATE_CMD_LIM_PCT * 0.3

#define CONFIG_PITCH_RATE_P_GAIN					7.5f
#define CONFIG_PITCH_RATE_I_GAIN					5.5f
#define CONFIG_PITCH_RATE_D_GAIN					0.0f
#define CONFIG_PITCH_RATE_D_LPF_CUTOFF_FREQ_HZ		75.0f
#define CONFIG_PITCH_RATE_CMD_LIM_PCT				5.0f
#define CONFIG_PITCH_RATE_I_CMD_LIM_PCT				CONFIG_PITCH_RATE_CMD_LIM_PCT * 0.3

#define CONFIG_YAW_RATE_P_GAIN						7.5f
#define CONFIG_YAW_RATE_I_GAIN						5.5f
#define CONFIG_YAW_RATE_D_GAIN						0.0f
#define CONFIG_YAW_RATE_D_LPF_CUTOFF_FREQ_HZ		75.0f
#define CONFIG_YAW_RATE_CMD_LIM_PCT					5.0f
#define CONFIG_YAW_RATE_I_CMD_LIM_PCT				CONFIG_YAW_RATE_CMD_LIM_PCT * 0.3

// MIXER----------------------------------------------------------------------
#define CONFIG_THRUST_COMP							ENABLED

/* SENSOR CONFIG SETTINGS----------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// IMU------------------------------------------------------------------------
#define LSM6DSOX_DEVICE_ID							0U
#define CONFIG_IMU_DEVICE							LSM6DSOX_DEVICE_ID

#define IMU_I2C_PROTOCOL_ID							0U
#define CONFIG_IMU_COMM_PROTOCOL					IMU_I2C_PROTOCOL_ID

#define CONFIG_GY_LPF								DISABLED
#define CONFIG_GY_LPF_CUTOFF_FREQ_RPS 				2 * PI * 500.0f

#define CONFIG_XL_LPF								DISABLED
#define CONFIG_XL_LPF_CUTOFF_FREQ_RPS 				2 * PI * 40.0f

/* PROTOCOL CONFIG SETTINGS--------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// PWM------------------------------------------------------------------------
#define CONFIG_PWM_PULSE_MIN_US 					988U
#define CONFIG_PWM_PULSE_MAX_US 					2012U
#define PWM_PULSE_MED_US							(CONFIG_PWM_PULSE_MIN_US + CONFIG_PWM_PULSE_MAX_US) / 2

#define PWM_PULSE_PROTO_MIN_US						1000U
#define PWM_PULSE_PROTO_MAX_US						2000U
#define PWM_PULSE_VALID_MIN_US						950U
#define PWM_PULSE_VALID_MAX_US						2050U

/* RX CONFIG SETTINGS--------------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// PROTOCOL-------------------------------------------------------------------
#define RX_PWM_PROTOCOL_ID							0U
#define CONFIG_RX_PROTOCOL							RX_PWM_PROTOCOL_ID

/* ESC CONFIG SETTINGS-------------------------------------------------------------
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
-----------------------------------------------------------------------------------*/
// PROTOCOL-------------------------------------------------------------------
#define ESC_PWM_PROTOCOL_ID							0U
#define CONFIG_ESC_PROTOCOL							ESC_PWM_PROTOCOL_ID

// COMMANDS-------------------------------------------------------------------
#define CONFIG_ESC_CMD_IDLE_PCT						18.0f
#define CONFIG_ESC_CMD_LIFTOFF_PCT					24.0f
#define CONFIG_ESC_CMD_LIMIT_PCT					100.0f // Max is 100% (this would unlock full motor potential)

/*
 * On Full Charged Battery
 *
 * 3036/3037 (lowest movement cmd)
 * 3055/3056 (lowest continuous spin speed cmd)
 * 3650 (liftoff cmd)
 *
 */
