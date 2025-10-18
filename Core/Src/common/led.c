/*
 * led.c
 *
 *  Created on: Oct 19, 2023
 *      Author: charlieroman
 */

#include "common/time.h"
#include "common/led.h"
#include "main.h"
#include "common/maths.h"

/*
 * @brief blink led at a specified frequency (Hz)
 * (must be called within a loop)
 *
 * @param  hz		frequency to blink led
 * @retval None
 */
void blink_led(uint16_t hz)
{
	uint32_t time, interval_us;
	static uint32_t prev_time;

	time = get_timestamp();
	interval_us = time - prev_time;

	if (interval_us > HZ_TO_INTERVAL_US(hz))
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		prev_time = time;
	}

}

/*
 * @brief toggle led (on/off)
 *
 * @param  request	on/off request
 * @retval None
 */
void toggle_led(uint8_t request)
{
	uint8_t led_request, led_status;
	led_request = request;
	led_status = HAL_GPIO_ReadPin(LED_GPIO_Port, LED_Pin);

	if (led_request != led_status)
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	}
}

/*
 * @brief set led status (controls led blink frequency)
 *
 * @param  status	ready, waiting, or error status
 * @retval None
 */
void led_status(uint8_t status)
{
	switch(status) {
		case READY:
			toggle_led(ON); //solid for ready
			break;

		case ERROR:
			blink_led(1); 	//1 Hz for error
			break;

		case WAITING:
			blink_led(5); 	//5 Hz for waiting
			break;

		default:
			toggle_led(OFF);
			break;
	}
}
