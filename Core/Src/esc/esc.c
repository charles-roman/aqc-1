/*
 * esc.c
 *
 *  Created on: Oct 14, 2025
 *      Author: charlieroman
 */

#include "esc/esc.h"
#include "esc/protocols/pwm_esc.h"
#include "common/maths.h"
#include "common/settings.h"

/**
  * @brief  ESC Protocol Setting
  */
#define ESC_PROTOCOL			CONFIG_ESC_PROTOCOL

/**
  * @brief  ESC Command Settings
  */
#define ESC_CMD_IDLE_PCT		CONFIG_ESC_CMD_IDLE_PCT
#define ESC_CMD_LIFTOFF_PCT		CONFIG_ESC_CMD_LIFTOFF_PCT
#define ESC_CMD_LIMIT_PCT		CONFIG_ESC_CMD_LIMIT_PCT

/**
  * @brief  ESC Commands Handle
  */
static esc_cmds_t cmd;

/**
  * @brief  ESC Command Properties Handle
  */
static esc_cmd_props_t cmd_props;

/**
  * @brief  ESC Driver Pointer for Protocol Interface
  */
static const esc_protocol_interface_t *esc_driver = NULL;


/**
  * @brief fetches esc command properties
  *
  * @param	out		esc command properties handle buffer to be filled
  * @retval None
  */
void esc_get_command_properties(esc_cmd_props_t *out) {
    *out = cmd_props;
}

/**
  * @brief calculates esc cmd based on pct
  *
  * @param  pct		percentage to set esc command at
  * @retval esc command
  */
static inline uint32_t map_pct_to_esc_cmd(float pct) {
	return (uint32_t) ((cmd_props.max - cmd_props.min) * (pct / 100.0f) + cmd_props.min);
}

/**
  * @brief helper function to validate/constrain esc command
  *
  * @param  cmd		pointer to esc command
  * @retval esc status
  */
static inline esc_status_t sanitize_esc_command(uint32_t *cmd) {
	if (!inrange_u32(*cmd, cmd_props.idle, cmd_props.limit)) {
		*cmd = constrain_u32(*cmd, cmd_props.idle, cmd_props.limit); // constrain within esc cmd range
		return ESC_ERROR_WARN;
	}
	return ESC_OK;
}

/**
  * @brief helper function to validate esc_driver initialization
  *
  * @param  driver	pointer to esc driver
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
			driver->set_commands);
}

/**
  * @brief esc API call to init esc protocol driver interface
  *
  * @retval esc status
  */
esc_status_t esc_init(void) {
	/* Use pre-processor conditionals based on configured protocol to initialize esc_driver.
	 * NOTE: hot-swaps would require ESC_PROTOCOL to be a modifiable variable */
	#if ESC_PROTOCOL == ESC_PWM_PROTOCOL_ID
		esc_driver = &pwm_esc_driver;
	#else
		#error "Invalid ESC protocol configuration"
	#endif

	if (!valid_esc_driver(esc_driver))
		return ESC_ERROR_FATAL;

	esc_status_t status = esc_driver->init(&cmd_props.min, &cmd_props.max);

	/* Init remaining motor command properties */
	cmd_props.idle = map_pct_to_esc_cmd(ESC_CMD_IDLE_PCT);
	if (!inrange_u32(cmd_props.idle, cmd_props.min, cmd_props.max))
		return ESC_ERROR_FATAL;

	cmd_props.liftoff = map_pct_to_esc_cmd(ESC_CMD_LIFTOFF_PCT);
	if (!inrange_u32(cmd_props.liftoff, cmd_props.min, cmd_props.max))
		return ESC_ERROR_FATAL;

	cmd_props.limit = map_pct_to_esc_cmd(ESC_CMD_LIMIT_PCT);
	if (!inrange_u32(cmd_props.limit, cmd_props.min, cmd_props.max))
		return ESC_ERROR_FATAL;

	/* Init motor command variables */
	cmd.esc1 = cmd_props.min;
	cmd.esc2 = cmd_props.min;
	cmd.esc3 = cmd_props.min;
	cmd.esc4 = cmd_props.min;

	return status;
}

/**
  * @brief esc API call to deinit esc protocol driver interface
  *
  * @retval esc status
  */
esc_status_t esc_deinit(void) {
	if (!esc_driver)
		return ESC_ERROR_WARN;

	/* Reset cached motor command properties */
	cmd_props.min = 0U;
	cmd_props.max = 0U;
	cmd_props.idle = 0U;
	cmd_props.liftoff = 0U;
	cmd_props.limit = 0U;

	/* Set motor command variables to safe state*/
	cmd.esc1 = cmd_props.min;
	cmd.esc2 = cmd_props.min;
	cmd.esc3 = cmd_props.min;
	cmd.esc4 = cmd_props.min;

	esc_status_t status = esc_driver->deinit();
	esc_driver = NULL;

	return status;
}

/**
  * @brief esc API call to start comms
  *
  * @retval esc status
  */
esc_status_t esc_start(void) {
	if (!esc_driver)
		return ESC_ERROR_FATAL;

	return esc_driver->start(cmd_props.min);
}

/**
  * @brief esc API call to stop comms
  *
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
  * @retval None
  */
void esc_arm(void) {
	if (!esc_driver)
		return ESC_ERROR_FATAL;

	esc_driver->arm(cmd_props.idle);
}

/**
  * @brief esc API call to disarm system (stops motors)
  *
  * @retval None
  */
void esc_disarm(void) {
	if (!esc_driver)
		return ESC_ERROR_FATAL;

	esc_driver->disarm(cmd_props.min);
}

/**
  * @brief esc API call to check whether esc is armed
  *
  * @retval boolean
  */
bool esc_is_armed(void) {
	if (cmd.esc1 >= cmd_props.idle)
		return true;

	if (cmd.esc2 >= cmd_props.idle)
		return true;

	if (cmd.esc3 >= cmd_props.idle)
		return true;

	if (cmd.esc4 >= cmd_props.idle)
		return true;

	return false;
}

/**
  * @brief esc API call to set motor commands
  *
  * @param  cmd		pointer to motor commands handle
  * @retval esc status
  */
esc_status_t esc_set_motor_commands(const mtr_cmds_t *mcmd) {
	esc_status_t status = ESC_OK;

	/* Cast to Integral Type */
	cmd.esc1 = (uint32_t) mcmd->mtr1;
	cmd.esc2 = (uint32_t) mcmd->mtr2;
	cmd.esc3 = (uint32_t) mcmd->mtr3;
	cmd.esc4 = (uint32_t) mcmd->mtr4;

	/* Validate Motor Commands */
	if (sanitize_esc_command(&cmd.esc1) != ESC_OK)
		status = ESC_ERROR_WARN;

	if (sanitize_esc_command(&cmd.esc2) != ESC_OK)
		status = ESC_ERROR_WARN;

	if (sanitize_esc_command(&cmd.esc3) != ESC_OK)
		status = ESC_ERROR_WARN;

	if (sanitize_esc_command(&cmd.esc4) != ESC_OK)
		status = ESC_ERROR_WARN;

	if (!esc_driver)
		return ESC_ERROR_FATAL;

	return esc_driver->set_commands(cmd);
}
