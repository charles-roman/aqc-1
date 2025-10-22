/*
 * esc.c
 *
 *  Created on: Oct 14, 2025
 *      Author: charlieroman
 */

#include "esc/esc.h"
#include "esc/protocols/pwm_esc.h"
#include "common/settings.h"

/**
  * @brief  Esc Protocol Setting
  */
#define ESC_PROTOCOL	CONFIG_ESC_PROTOCOL

/**
  * @brief  esc driver pointer for protocol interface
  */
static const esc_protocol_interface_t *esc_driver = NULL;

/**
  * @brief helper function to validate esc_driver initialization
  *
  * @param  driver	pointer to esc driver
  *
  * @retval boolean
  */
static bool valid_esc_driver(esc_protocol_interface_t *driver) {
	return (driver &&
			driver->init &&
		    driver->deinit &&
			driver->start &&
			driver->stop &&
			driver->arm &&
			driver->disarm &&
			driver->is_armed &&
			driver->set_motor_commands);
}

/**
  * @brief esc API call to init esc protocol driver interface and start comms
  *
  * @param  None
  * @retval esc status
  */
esc_status_t esc_init(void) {
	/* Use pre-processor conditionals based on configured protocol to initialize esc_driver.
	 * NOTE: hot-swaps would require ESC_PROTOCOL to be a modifiable variable */
	#if ESC_PROTOCOL == ESC_PWM_PROTOCOL_ID
	esc_driver = &pwm_esc_driver;
	#else
	return ESC_ERROR_FATAL;
	#endif

	if (!valid_esc_driver(esc_driver))
		return ESC_ERROR_FATAL;

	return esc_driver->init();
}

/**
  * @brief esc API call to deinit esc protocol driver interface
  *
  * @param  None
  * @retval None
  */
esc_status_t esc_deinit(void) {
	if (!esc_driver)
		return ESC_ERROR_WARN;

	esc_driver->deinit();
	esc_driver = NULL;

	return ESC_OK;
}

/**
  * @brief esc API call to start comms
  *
  * @param  None
  * @retval esc status
  */
esc_status_t esc_start(void) {
	if (!esc_driver)
		return ESC_ERROR_FATAL;

	return esc_driver->start();
}

/**
  * @brief esc API call to stop comms
  *
  * @param  None
  * @retval esc status
  */
esc_status_t esc_stop(void) {
	if (!esc_driver)
		return ESC_ERROR_FATAL;

	return esc_driver->stop();
}

/**
  * @brief esc API call to arm system (idles motors)
  *
  * @param  None
  * @retval None
  */
void esc_arm(void) {
	if (!esc_driver)
		return ESC_ERROR_FATAL;

	return esc_driver->arm();
}

/**
  * @brief esc API call to disarm system (stops motors)
  *
  * @param  None
  * @retval None
  */
void esc_disarm(void) {
	if (!esc_driver)
		return ESC_ERROR_FATAL;

	return esc_driver->disarm();
}


/**
  * @brief esc API call to check whether esc is armed
  *
  * @param  None
  * @retval None
  */
bool esc_is_armed(void) {
	if (!esc_driver)
		return ESC_ERROR_FATAL;

	return esc_driver->is_armed();
}

/**
  * @brief esc API call to set motor commands
  *
  * @param  cmd		pointer to motor commands handle
  *
  * @retval esc status
  */
esc_status_t esc_set_motor_commands(const mtr_cmds_t *cmd) {
	/* NOTE: include this check when integrating hot-swaps or other features which may
	 * call esc_deinit during runtime, otherwise leave lightweight due to call frequency
	 *
	 * if (!esc_driver) return ESC_ERROR_FATAL; */

	return esc_driver->set_motor_commands(cmd);
}
