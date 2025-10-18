/*
 * esc.c
 *
 *  Created on: Oct 14, 2025
 *      Author: charlieroman
 */

#include "esc.h"
#include "protocols/pwm_esc.h"
#include "../common/settings.h"

/**
  * @brief  Esc Protocol Setting
  */
#define ESC_PROTOCOL	CONFIG_ESC_PROTOCOL

/**
  * @brief  esc driver pointer for protocol interface
  */
static const esc_protocol_interface_t *esc_driver = NULL;

/**
  * @brief esc API call to init esc protocol driver interface and start comms
  *
  * @param  None
  * @retval None
  */
void esc_init(void) {
	/* NOTE: use pre-processor conditionals based
	* on configured protocol to initialize esc_driver */
	#if ESC_PROTOCOL == ESC_PWM_PROTOCOL_ID
	esc_driver = &pwm_driver;
	#endif

	esc_driver->init();
	esc_driver->start();
}

/**
  * @brief esc API call to stop comms
  *
  * @param  None
  * @retval None
  */
void esc_deinit(void) {
	if (esc_driver && esc_driver->stop)
		esc_driver->stop();
}

/**
  * @brief esc API call to arm quad-copter (idles motors)
  *
  * @param  None
  * @retval None
  */
void arm(void) {
	esc_driver->arm();
}

/**
  * @brief esc API call to disarm quad-copter (stops motors)
  *
  * @param  None
  * @retval None
  */
void disarm(void) {
	esc_driver->disarm();
}

/**
  * @brief esc API call to set motor commands
  *
  * @param  cmd		pointer to motor commands handle
  *
  * @retval None
  */
void set_motor_commands(mtr_cmds_t *cmd) {
	esc_driver->set_motor_commands(cmd);
}
