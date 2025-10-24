/*
 * rc_input.h
 *
 *  Created on: Oct 19, 2025
 *      Author: charlieroman
 */

#pragma once

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  RC Request Status Type
  */
typedef enum {
    RC_REQ_OK			= 0x00U,
    RC_REQ_ERROR_WARN	= 0x01U,
	RC_REQ_ERROR_FATAL	= 0x02U
} rc_req_status_t;

/**
  * @brief  Flight Mode Status Type
  */
typedef enum {
	INVALID_MODE	= 0x00U,
	ANGLE_MODE		= 0x01U,
	RATE_MODE		= 0x02U
} mode_status_t;

/**
  * @brief  RC State Request Handle
  */
typedef struct rc_reqs {
	float roll;
	float pitch;
	float throttle;
	float yaw;
} rc_reqs_t;

/* Exported functions prototypes ---------------------------------------------*/
rc_req_status_t rc_get_requests(rc_reqs_t *req);

rc_req_status_t rc_init(void);

rc_req_status_t rc_deinit(void);

mode_status_t rc_get_flight_mode(void);

bool rc_is_armed(void);

bool rc_is_throttle_idle(void);
