/*
 * rx.c
 *
 *  Created on: Oct 7, 2025
 *      Author: charlieroman
 */

#include "rx.h"
#include "pwm_rx.h"

/**
  * @brief determines if quad is currently armed
  *
  * @param None
  * @retval ret		logical value (1 or 0)
  */
bool is_armed(GPIO_TypeDef* ARM_GPIO_Port, uint16_t ARM_GPIO_Pin) {
	bool ret;

	/* Determine if arm switch (SE) is active */
	ret = (HAL_GPIO_ReadPin(ARM_GPIO_Port, ARM_GPIO_Pin) == GPIO_PIN_SET);

	return ret;
}

/**
  * @brief init rx communication (based on protocol)
  *
  * @param  None
  * @retval None
  */
void init_rx_comm_protocol(void) {
	init_ic_timclk_ref_props();
	init_pwm_pulse_handles();
}

/**
  * @brief start rx communication (based on protocol)
  *
  * @param  None
  * @retval None
  */
void start_rx_comm_capture(void) {
	start_pwm_input_capture();
}

/**
  * @brief update user request (based on protocol)
  *
  * @param  st	pointer to system state handle
  * @retval None
  */
void get_rc_request(systemState *st) {
	get_rc_requests_over_pwm(st);
}
