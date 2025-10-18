/*
 * rx.c
 *
 *  Created on: Oct 7, 2025
 *      Author: charlieroman
 */

#include "rx/rx.h"
#include "rx/protocols/pwm_rx.h"

void rx_init(void) {

}

/**
  * @brief determines if quad is currently armed
  *
  * @param None
  * @retval ret		logical value (1 or 0)
  */
bool is_armed(GPIO_TypeDef* ARM_GPIO_PORT, uint16_t ARM_GPIO_PIN) {
	bool ret;

	/* Determine if arm switch (SE) is active */
	ret = (HAL_GPIO_ReadPin(ARM_GPIO_PORT, ARM_GPIO_PIN) == GPIO_PIN_SET);

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
void get_rc_requests(rc_reqs_t *req) {
	get_rc_requests_over_pwm(req);
}
