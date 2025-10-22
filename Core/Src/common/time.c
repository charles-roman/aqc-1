/*
 * timer.c
 *
 *  Created on: Oct 15, 2023
 *      Author: charlieroman
 */

#include "common/time.h"

/*
 * @brief  computes and returns desired timer clock reference frequency
 *
 * @param  htim       pointer to HAL timer handle
 * @retval freq (hz); 0 if invalid
 */
uint32_t Get_TIMxClkRefFreqHz(TIM_HandleTypeDef *htim) {
	uint32_t APB_PCLK_FREQ_HZ, APB_TIMCLK_FREQ_HZ, PSC, TIMx_ClkRefFreqHz;

	if (htim == NULL)
		return 0; // Error

	if (htim->Instance == TIM8) {
		/* Get APB2 Clock Freq */
		APB_PCLK_FREQ_HZ = HAL_RCC_GetPCLK2Freq();

		/* Get APB2 Timer Clock Freq */
		APB_TIMCLK_FREQ_HZ = (RCC->CFGR & RCC_CFGR_PPRE2_Msk) != RCC_CFGR_PPRE2_DIV1 ?
		    				  APB_PCLK_FREQ_HZ * 2 : APB_PCLK_FREQ_HZ; // doubled if PSC > 1

	} else if ((htim->Instance == TIM2) ||
			   (htim->Instance == TIM3) ||
			   (htim->Instance == TIM4) ||
			   (htim->Instance == TIM6)) {
		/* Get APB1 Clock Freq */
		APB_PCLK_FREQ_HZ = HAL_RCC_GetPCLK1Freq();

	    /* Get APB1 Timer Clock Freq */
	    APB_TIMCLK_FREQ_HZ = (RCC->CFGR & RCC_CFGR_PPRE1_Msk) != RCC_CFGR_PPRE1_DIV1 ?
	    					  APB_PCLK_FREQ_HZ * 2 : APB_PCLK_FREQ_HZ; // doubled if PSC > 1

	} else {
		return 0; // Error

	}

	/* Get Pre-scaler */
	PSC = htim->Init.Prescaler + 1;

	/* Error Checks */
	if ((APB_TIMCLK_FREQ_HZ == 0) || (PSC > APB_TIMCLK_FREQ_HZ))
		return 0; // Error

	/* Compute Timer Clock Reference Freq */
	TIMx_ClkRefFreqHz = APB_TIMCLK_FREQ_HZ / PSC;

	return TIMx_ClkRefFreqHz;
}

/*
 * @brief  computes and returns desired timer clock reference frequency
 *
 * @param  htim       pointer to HAL timer handle
 * @retval freq (mhz); 0 if unable to be calculated in mhz as an integral type
 */
uint32_t Get_TIMxClkRefFreqMHz(TIM_HandleTypeDef *htim) {
	const uint32_t ONE_MILLION = 1000000U;

	/* Get Timer Clock Reference Freq in Hz */
	uint32_t TIMx_ClkRefFreqHz = Get_TIMxClkRefFreqHz(htim);
	if (TIMx_ClkRefFreqHz == 0)
		return 0;

	/* Check if Divisible by 1 Million */
	if (TIMx_ClkRefFreqHz % ONE_MILLION != 0)
		return 0;

	return TIMx_ClkRefFreqHz / ONE_MILLION;
}

/*
 * @brief  start specified timer peripheral
 *
 * @param  htim		pointer to HAL timer handle
 * @retval None
 */
void start_timer(TIM_HandleTypeDef *htim) {
	HAL_TIM_Base_Start(htim);
}

/*
 * @brief  stop specified timer peripheral
 *
 * @param  htim		pointer to HAL timer handle
 * @retval None
 */
void stop_timer(TIM_HandleTypeDef *htim) {
	HAL_TIM_Base_Stop(htim);
}

/*
 * @brief  delay mcu operation for specified time period (ms)
 *
 * @param  ms       delay in ms
 * @retval None
 */
void delay(uint32_t ms) {
  HAL_Delay(ms);
}

/*
 * @brief  provides a tick value from system clock
 *
 * @param  None
 * @retval tick value (ms)
 */
uint32_t millis(void) {
	return HAL_GetTick();
}
