/*
 * timer.c
 *
 *  Created on: Oct 15, 2023
 *      Author: charlieroman
 */

#include "main.h"
#include "maths.h"
#include "time.h"

/*
 * @brief  delay
 *
 * @param  ms       delay in ms
 * @retval None
 */
void delay(uint32_t ms)
{
  HAL_Delay(ms);
}

/*
 * @brief  timer start
 *
 * @param  htim		pointer to HAL timer struct
 * @retval None
 */
void start_timer(TIM_HandleTypeDef *htim)
{
	HAL_TIM_Base_Start(htim);
}

/*
 * @brief  timer stop
 *
 * @param  htim		pointer to HAL timer struct
 * @retval None
 */
void stop_timer(TIM_HandleTypeDef *htim)
{
	HAL_TIM_Base_Stop(htim);
}

/*
 * @brief  program run time
 *
 * @param  None
 * @retval 			program runtime in ms
 */
__weak uint32_t millis(void) //
{
	return HAL_GetTick();
}

/*
 * @brief  program run time
 *
 * @param  None
 * @retval 			program runtime in us
 */
uint32_t get_timestamp(void)
{
	static uint32_t timestamp;
	uint16_t dt, count, ref_clk_mhz;

	/* Determine Reference Clock Freq */
	ref_clk_mhz = APB1_CLK_FREQ_MHZ/(htim6.Init.Prescaler + 1);

	/* Get Counter Value */
	count = __HAL_TIM_GET_COUNTER(&htim6);

	/* Determine Timestep Since Last Call */
	dt = count / (ref_clk_mhz);

	/* Add Timestep to Running Total */
	timestamp += dt;

	/* Reset Counter */
	__HAL_TIM_SET_COUNTER(&htim6, 0);

	return timestamp;
}
