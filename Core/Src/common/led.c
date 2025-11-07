/*
 * led.c
 *
 *  Created on: Oct 19, 2023
 *      Author: charlieroman
 */

#include "common/led.h"
#include "common/time.h"
#include "common/maths.h"
#include "common/hardware.h"

/*
 * @brief turn led on
 *
 * @retval None
 */
void led_on(void) {
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

/*
 * @brief turn led off
 *
 * @retval None
 */
void led_off(void) {
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

/*
 * @brief toggle led (on/off)
 *
 * @retval None
 */
void led_toggle(void) {
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

/*
 * @brief blink led at a specified frequency (0 - 1000Hz)
 * 		  (must be called within a loop)
 *
 * @param  hz		frequency to blink led (hz)
 * @retval None
 */
void led_blink(uint16_t hz) {
	uint32_t now, dt_ms, period_ms;
	static uint32_t last_tick;

	/* Constrain Input */
	if (hz > 1000U)
		hz = 1000U;

	/* Get Elapsed Time Interval */
	now = millis();
	dt_ms = now - last_tick;
	period_ms = HZ_TO_INTERVAL_MS(hz);

	/* Toggle LED (if time) */
	if (dt_ms >= period_ms / 2) {
		led_toggle();
		last_tick = now;
	}

}

/*
 * @brief set led status (controls led blink frequency)
 *
 * @param  status	led status type
 * @retval None
 */
void led_set_status(led_status_t status) {
	switch(status) {
		case LED_READY:
			led_on(); 		// solid for ready
			break;

		case LED_ERROR:
			led_blink(1); 	// 1 Hz for error
			break;

		case LED_WAITING:
			led_blink(5); 	// 5 Hz for waiting
			break;

		default:
			led_off();
			break;
	}
}
