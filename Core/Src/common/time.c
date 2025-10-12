/*
 * timer.c
 *
 *  Created on: Oct 15, 2023
 *      Author: charlieroman
 */

#include "time.h"

/* Timer Clock Reference Freqs */
static uint32_t TIM2_CLK_REF_FREQ_HZ;
static uint32_t TIM3_CLK_REF_FREQ_HZ;
static uint32_t TIM4_CLK_REF_FREQ_HZ;
static uint32_t TIM6_CLK_REF_FREQ_HZ;
static uint32_t TIM8_CLK_REF_FREQ_HZ;

/*
 * @brief  get peripheral bus 1 timer clock freq
 *
 * @param  None
 * @retval freq (hz)
 */
static uint32_t Get_APB1_TIMCLKFreq(void) {
	uint32_t APB1_PCLK_FREQ_HZ, APB1_TIMCLK_FREQ_HZ;

	/* Get APB1 Clock Freq */
    APB1_PCLK_FREQ_HZ = HAL_RCC_GetPCLK1Freq();

    /* Get APB1 Timer Clock Freq */
    APB1_TIMCLK_FREQ_HZ = (RCC->CFGR & RCC_CFGR_PPRE1_Msk) != RCC_CFGR_PPRE1_DIV1 ?
    					  APB1_PCLK_FREQ_HZ * 2 : APB1_PCLK_FREQ_HZ; // doubled if PSC > 1

    return APB1_TIMCLK_FREQ_HZ;
}

/*
 * @brief  get peripheral bus 2 timer clock freq
 *
 * @param  None
 * @retval freq (hz)
 */
static uint32_t Get_APB2_TIMCLKFreq(void) {
	uint32_t APB2_PCLK_FREQ_HZ, APB2_TIMCLK_FREQ_HZ;

	/* Get APB1 Clock Freq */
    APB2_PCLK_FREQ_HZ = HAL_RCC_GetPCLK2Freq();

    /* Get APB1 Timer Clock Freq */
    APB2_TIMCLK_FREQ_HZ = (RCC->CFGR & RCC_CFGR_PPRE2_Msk) != RCC_CFGR_PPRE2_DIV1 ?
    					  APB2_PCLK_FREQ_HZ * 2 : APB2_PCLK_FREQ_HZ; // doubled if PSC > 1

    return APB2_TIMCLK_FREQ_HZ;
}

/*
 * @brief  caches desired timer clock reference frequency for future use
 *
 * @param  htim       pointer to HAL timer handle
 * @retval None
 */
void Cache_TIMCLKRefFreq(TIM_HandleTypeDef *htim) {
	if (htim == NULL)
		return; // Setup_Error_Handler();

	if (htim->Instance == TIM2) {
		TIM2_CLK_REF_FREQ_HZ = Get_APB1_TIMCLKFreq() / (htim->Init.Prescaler + 1);

	} else if (htim->Instance == TIM3) {
		TIM3_CLK_REF_FREQ_HZ = Get_APB1_TIMCLKFreq() / (htim->Init.Prescaler + 1);

	} else if (htim->Instance == TIM4) {
		TIM4_CLK_REF_FREQ_HZ = Get_APB1_TIMCLKFreq() / (htim->Init.Prescaler + 1);

	} else if (htim->Instance == TIM6) {
		TIM6_CLK_REF_FREQ_HZ = Get_APB1_TIMCLKFreq() / (htim->Init.Prescaler + 1);

	} else if (htim->Instance == TIM8) {
		TIM8_CLK_REF_FREQ_HZ = Get_APB2_TIMCLKFreq() / (htim->Init.Prescaler + 1);

	} else {
		// Setup_Error_Handler();
	}

}

/*
 * @brief  retrieves the timer 2 clock reference freq
 *
 * @param  None
 * @retval freq (hz)
 */
uint32_t Get_TIM2CLKRefFreqHz(void) {
	return TIM2_CLK_REF_FREQ_HZ;
}

/*
 * @brief  retrieves the timer 3 clock reference freq
 *
 * @param  None
 * @retval freq (hz)
 */
uint32_t Get_TIM3CLKRefFreqHz(void) {
    return TIM3_CLK_REF_FREQ_HZ;
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
