/*
 * rc_input.c
 *
 *  Created on: Oct 19, 2025
 *      Author: charlieroman
 */

#include <stdint.h>
#include <stdbool.h>
#include "flight/rc_input.h"
#include "rx/rx.h"
#include "common/maths.h"
#include "common/settings.h"

/**
  * @brief  Rx Protocol Setting
  */
#define RX_PROTOCOL					CONFIG_RX_PROTOCOL

/**
  * @brief  PWM Config Settings
  */
#define PWM_PULSE_MIN_US    		CONFIG_PWM_PULSE_MIN_US
#define PWM_PULSE_MAX_US    		CONFIG_PWM_PULSE_MAX_US
#define PWM_PULSE_MED_US			(PWM_PULSE_MIN_US + PWM_PULSE_MAX_US) / 2
#define PWM_PULSE_PROTO_MIN_US		CONFIG_PWM_PULSE_PROTO_MIN_US
#define PWM_PULSE_PROTO_MAX_US		CONFIG_PWM_PULSE_PROTO_MAX_US
#define PWM_PULSE_VALID_MIN_US 		CONFIG_PWM_PULSE_VALID_MIN_US
#define PWM_PULSE_VALID_MAX_US		CONFIG_PWM_PULSE_VALID_MAX_US

/**
  * @brief  RC Input Settings
  */
#define ROLL_MIN_DEG				CONFIG_ROLL_MIN_DEG
#define ROLL_MAX_DEG	 			CONFIG_ROLL_MAX_DEG
#define ROLL_MIN_DPS				CONFIG_ROLL_MIN_DPS
#define ROLL_MAX_DPS				CONFIG_ROLL_MAX_DPS
#define PITCH_MIN_DEG				CONFIG_PITCH_MIN_DEG
#define PITCH_MAX_DEG	 			CONFIG_PITCH_MAX_DEG
#define PITCH_MIN_DPS				CONFIG_PITCH_MIN_DPS
#define PITCH_MAX_DPS	 			CONFIG_PITCH_MAX_DPS
#define YAW_MIN_DPS					CONFIG_YAW_MIN_DPS
#define YAW_MAX_DPS	 	 			CONFIG_YAW_MAX_DPS
#define THROTTLE_MIN_PCT			CONFIG_THROTTLE_MIN_PCT
#define THROTTLE_MAX_PCT			CONFIG_THROTTLE_MAX_PCT

#define THROTTLE_IDLE_TOLERANCE_PCT	CONFIG_THROTTLE_IDLE_TOLERANCE_PCT

/**
  * @brief  AETR RC Channel Type (describes map to rx channel)
  * 		NOTE: this is only for AETR convention!
  */
typedef enum {
	INVALID_CHANNEL		= 0x00U,
	ROLL_CHANNEL		= 0x01U,
	PITCH_CHANNEL		= 0x02U,
	THROTTLE_CHANNEL	= 0x03U,
	YAW_CHANNEL			= 0x04U,
	ARM_CHANNEL			= 0x05U,	// common channel choice for ARM
	MODE_CHANNEL		= 0x06U		// common channel choice for FLIGHT MODE
} aetr_rc_channel_t;

/**
  * @brief  ARM Status Type
  */
typedef enum {
	DISARMED = 1U,
	ARMED	 = 2U
} arm_status_t;

/**
  * @brief  Interface for Channel Mapping
  */
static rc_req_status_t (*map_channel_to_state_request)(aetr_rc_channel_t ch, uint32_t val, float *req) = NULL;

/**
  * @brief  State Request Variable(s)
  */
static float throttle_request = 0.0f;

/**
  * @brief map pwm pulse to valid state variable request
  *
  * @param ch	rc channel
  * @param val	channel value to map
  * @param req	request buffer to stored mapped value
  *
  * @retval rc request status
  */
static rc_req_status_t map_pulse_to_state_request(aetr_rc_channel_t ch, uint32_t val, float *req) {
	rc_req_status_t req_status = RC_REQ_OK;
	mode_status_t mode_status;

	/* Sanitize Pulse Width */
	if (!inrange_u32(val, PWM_PULSE_VALID_MIN_US, PWM_PULSE_VALID_MAX_US)) {
		req_status = RC_REQ_ERROR_WARN;
		val = (ch == THROTTLE_CHANNEL) ? PWM_PULSE_MIN_US : PWM_PULSE_MED_US; // set pulse width to neutral state

	} else if (!inrange_u32(val, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US)) {
		val = constrain_u32(val, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US);
	}

	/* Get Mode Status */
	mode_status = rc_get_mode_status();

	/* Map Pulse Width and Update Request */
	switch (ch) {
		case (ROLL_CHANNEL):
			if (mode_status == ANGLE_MODE) {
				*req = mapf((float) val, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US, ROLL_MIN_DEG, ROLL_MAX_DEG);

			} else if (mode_status == RATE_MODE) {
				*req = mapf((float) val, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US, ROLL_MIN_DPS, ROLL_MAX_DPS);

			} else { // Error: undefined flight mode
				req_status = RC_REQ_ERROR_WARN;
				// leave req buffer alone (maintains last state)
			}

			break;

		case (PITCH_CHANNEL):
			if (mode_status == ANGLE_MODE) {
				*req = mapf((float) val, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US, PITCH_MIN_DEG, PITCH_MAX_DEG);

			} else if (mode_status == RATE_MODE) {
				*req = mapf((float) val, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US, PITCH_MIN_DPS, PITCH_MAX_DPS);

			} else { // Error: undefined flight mode
				req_status = RC_REQ_ERROR_WARN;
				// leave req buffer alone (maintains last state)
			}

			break;

		case (THROTTLE_CHANNEL):
			throttle_request = mapf((float) val, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US, THROTTLE_MIN_PCT, THROTTLE_MAX_PCT);	// store throttle_request for rc_is_throttle_idle check
			*req = throttle_request;
			break;

		case (YAW_CHANNEL):
			*req = mapf((float) val, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US, YAW_MIN_DPS, YAW_MAX_DPS);
			break;

		default:	// Error: unexpected channel mapping
			req_status = RC_REQ_ERROR_WARN;
			break;
	}

	return req_status;
}

/**
  * @brief initialize rc input mapping function
  *
  * @retval rc request status
  */
rc_req_status_t rc_init(void) {
	#if RX_PROTOCOL == RX_PWM_PROTOCOL_ID
	map_channel_to_state_request = map_pulse_to_state_request;
	return RC_REQ_OK;

	#else
	return RC_REQ_ERROR_FATAL
	#endif
}

/**
  * @brief de-initialize rc input mapping function
  * 	   (NOTE: included for clarity when adding rx protocol hot-swaps)
  *
  * @retval rc request status
  */
rc_req_status_t rc_deinit(void) {
	map_channel_to_state_request = NULL;
	return RC_REQ_OK;
}

/**
  * @brief map rx input and update request fields of rc request handle
  *
  * @param  req		pointer to rc requests handle
  * @retval rc request status
  */
rc_req_status_t rc_get_requests(rc_reqs_t *req) {
	uint32_t val;
	rc_req_status_t status = RC_REQ_OK;

	if (map_channel_to_state_request == NULL)
		return RC_REQ_ERROR_FATAL;

	val = rx_get_channel(ROLL_CHANNEL); // Note: the return value can be validated if the channel may be undefined
	if (map_channel_to_state_request(ROLL_CHANNEL, val, &(req->roll)) != RC_REQ_OK)
		status = RC_REQ_ERROR_WARN;

	val = rx_get_channel(PITCH_CHANNEL); // Note: the return value can be validated if the channel may be undefined
	if (map_channel_to_state_request(PITCH_CHANNEL, val, &(req->pitch)) != RC_REQ_OK)
		status = RC_REQ_ERROR_WARN;

	val = rx_get_channel(THROTTLE_CHANNEL); // Note: the return value can be validated if the channel may be undefined
	if (map_channel_to_state_request(THROTTLE_CHANNEL, val, &(req->throttle)) != RC_REQ_OK)
		status = RC_REQ_ERROR_WARN;

	val = rx_get_channel(YAW_CHANNEL); // Note: the return value can be validated if the channel may be undefined
	if (map_channel_to_state_request(YAW_CHANNEL, val, &(req->yaw)) != RC_REQ_OK)
		status = RC_REQ_ERROR_WARN;

	return status;
}

/**
  * @brief gets current flight mode request from rc
  *
  * @retval flight mode
  */
mode_status_t rc_get_flight_mode(void) {
	return (mode_status_t) rx_get_channel(MODE_CHANNEL);
}

/**
  * @brief helper function to get current arm status
  * 	   NOTE: there is current no error handling for invalid channel here
  *
  * @retval arm status
  */
static inline arm_status_t rc_get_arm_status(void) {
	return (arm_status_t) rx_get_channel(ARM_CHANNEL);
}

/**
  * @brief checks whether rc is currently armed
  *
  * @retval boolean
  */
bool rc_is_armed(void) {
	if (rc_get_arm_status() == ARMED)
		return true;

	return false;
}

/**
  * @brief determines if throttle is in idle position
  *
  * @retval ret		boolean
  */
bool rc_is_throttle_idle(void) {
	/* Determine if throttle is within idle tolerance */
	return (throttle_request <= THROTTLE_IDLE_TOLERANCE_PCT);
}
