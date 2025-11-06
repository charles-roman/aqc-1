/*
 * pwm_esc.c
 *
 *  Created on: Oct 14, 2025
 *      Author: charlieroman
 */

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "esc/protocols/pwm_esc.h"
#include "common/time.h"
#include "common/maths.h"
#include "common/hardware.h"
#include "common/settings.h"

/**
  * @brief  PWM Config Settings
  */
#define PWM_PULSE_MIN_US 			CONFIG_PWM_PULSE_MIN_US
#define PWM_PULSE_MAX_US 			CONFIG_PWM_PULSE_MAX_US
#define PWM_PULSE_PROTO_MIN_US 		CONFIG_PWM_PULSE_PROTO_MIN_US
#define PWM_PULSE_PROTO_MAX_US 		CONFIG_PWM_PULSE_PROTO_MAX_US
#define PWM_PULSE_VALID_MIN_US 		CONFIG_PWM_PULSE_VALID_MIN_US
#define PWM_PULSE_VALID_MAX_US		CONFIG_PWM_PULSE_VALID_MAX_US

/**
  * @brief  ESC Channel -> Timer Aliases
  */
#define ESC1_PWM_OUT_TIM			TIM4
#define ESC2_PWM_OUT_TIM			TIM4
#define ESC3_PWM_OUT_TIM			TIM8
#define ESC4_PWM_OUT_TIM			TIM8

/**
  * @brief  ESC Channel -> Timer Channel Aliases
  */
#define ESC3_PWM_OUT_TIM_CHANNEL	TIM_CHANNEL_1
#define ESC4_PWM_OUT_TIM_CHANNEL	TIM_CHANNEL_2
#define ESC2_PWM_OUT_TIM_CHANNEL	TIM_CHANNEL_3
#define ESC1_PWM_OUT_TIM_CHANNEL	TIM_CHANNEL_4

/**
  * @brief  ESC Status Type Aliases
  */
#define PWM_ESC_OK					ESC_OK
#define PWM_ESC_ERROR_WARN			ESC_ERROR_WARN
#define PWM_ESC_ERROR_FATAL			ESC_ERROR_FATAL

typedef esc_status_t pwm_esc_status_t;

/**
  * @brief  Timer Handle Pointers
  * 		NOTE: Adjust based on PWM Output Timer Config!
  */
static const TIM_HandleTypeDef* phtim_esc1 = NULL;
static const TIM_HandleTypeDef* phtim_esc2 = NULL;
static const TIM_HandleTypeDef* phtim_esc3 = NULL;
static const TIM_HandleTypeDef* phtim_esc4 = NULL;

/**
  * @brief  PWM Output Timer Clock Reference Freq
  */
static uint32_t PWM_OUT_TIMxClkRefFreqMHz;


/**
  * @brief  wraps __HAL_TIM_SET_COMPARE macro
  */
#define SET_DUTY_CYCLE(htim, channel, value) \
    __HAL_TIM_SET_COMPARE((htim), (channel), (value))

/**
  * @brief  wraps HAL_TIM_PWM_Start function
  *
  * @param  htim	pointer to HAL timer handle
  * @param  channel timer channel
  *
  * @retval HAL status
  */
static inline HAL_StatusTypeDef PWM_Start_Channel(TIM_HandleTypeDef *htim, uint32_t channel) {
	return HAL_TIM_PWM_Start(htim, channel);
}

/**
  * @brief  wraps HAL_TIM_PWM_Stop function
  *
  * @param  htim	pointer to HAL timer handle
  * @param  channel timer channel
  *
  * @retval HAL status
  */
static inline HAL_StatusTypeDef PWM_Stop_Channel(TIM_HandleTypeDef *htim, uint32_t channel) {
	 return HAL_TIM_PWM_Stop(htim, channel);
}

/**
  * @brief helper function to get the appropriate timer handle based on hardware config
  *
  * @param  tim		pointer to timer type handle
  * @retval pointer to timer handle type (NULL otherwise)
  */
static TIM_HandleTypeDef* Get_ESC_PWM_OUT_TIM_Handle(const TIM_TypeDef* tim) {
	#if HTIM4 == CONFIGURED
	if (tim == TIM4)
		return &htim4;
	#endif

	#if HTIM8 == CONFIGURED
	if (tim == TIM8)
		return &htim8;
	#endif

	// add more as needed

	return NULL;
}

/**
  * @brief validates pwm output timer config(s)
  * 	   NOTE: timer config settings are important to verify if signals are
  * 	   generated from separate timers, otherwise just the pwm output frequency
  *
  * @retval boolean
  */
static bool valid_pwm_timer_config(void) {
	uint32_t ESC1_PWM_TIMClkRefFreqHz, \
			 ESC2_PWM_TIMClkRefFreqHz, \
			 ESC3_PWM_TIMClkRefFreqHz, \
			 ESC4_PWM_TIMClkRefFreqHz;

	uint32_t ESC1_PWM_TIM_CLK_ARR, \
			 ESC2_PWM_TIM_CLK_ARR, \
			 ESC3_PWM_TIM_CLK_ARR, \
			 ESC4_PWM_TIM_CLK_ARR;

	float PWM_OUT_FREQ_HZ;

	/* Validate pwm timer clock reference freqs */
	ESC1_PWM_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_esc1);
	ESC2_PWM_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_esc2);
	ESC3_PWM_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_esc3);
	ESC4_PWM_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_esc4);

	if (!all_equal_u32(ESC1_PWM_TIMClkRefFreqHz, \
				   	   ESC2_PWM_TIMClkRefFreqHz, \
					   ESC3_PWM_TIMClkRefFreqHz, \
					   ESC4_PWM_TIMClkRefFreqHz)) {
		return false;
	}

	/* Validate pwm timer clock ARRs */
	ESC1_PWM_TIM_CLK_ARR = phtim_esc1->Init.Period;
	ESC2_PWM_TIM_CLK_ARR = phtim_esc2->Init.Period;
	ESC3_PWM_TIM_CLK_ARR = phtim_esc3->Init.Period;
	ESC4_PWM_TIM_CLK_ARR = phtim_esc4->Init.Period;

	if (!all_equal_u32(ESC1_PWM_TIM_CLK_ARR, \
				   	   ESC2_PWM_TIM_CLK_ARR, \
					   ESC3_PWM_TIM_CLK_ARR, \
					   ESC4_PWM_TIM_CLK_ARR)) {
		return false;
	}

	// NOTE: Might also want to validate polarity, mode, etc.

	/* Validate pwm output freq */
	PWM_OUT_FREQ_HZ = (float) ESC1_PWM_TIMClkRefFreqHz / ESC1_PWM_TIM_CLK_ARR;

	if (HZ_TO_INTERVAL_US(PWM_OUT_FREQ_HZ) < (float) PWM_PULSE_VALID_MAX_US) {
		return false;
	}

	return true;
}

/**
  * @brief init pwm protocol config properties
  *
  * @param	esc_cmd_min		pointer to minimum esc command
  * @param 	esc_cmd_max		pointer to maximum esc command
  *
  * @retval pwm esc status
  */
static pwm_esc_status_t pwm_esc_init(uint32_t *esc_cmd_min, uint32_t *esc_cmd_max) {
	phtim_esc1 = Get_ESC_PWM_OUT_TIM_Handle(ESC1_PWM_OUT_TIM);
	if (phtim_esc1 == NULL)
		return PWM_ESC_ERROR_FATAL;

	phtim_esc2 = Get_ESC_PWM_OUT_TIM_Handle(ESC2_PWM_OUT_TIM);
	if (phtim_esc2 == NULL)
		return PWM_ESC_ERROR_FATAL;

	phtim_esc3 = Get_ESC_PWM_OUT_TIM_Handle(ESC3_PWM_OUT_TIM);
	if (phtim_esc3 == NULL)
		return PWM_ESC_ERROR_FATAL;

	phtim_esc4 = Get_ESC_PWM_OUT_TIM_Handle(ESC4_PWM_OUT_TIM);
	if (phtim_esc4 == NULL)
		return PWM_ESC_ERROR_FATAL;

	/* Validate pwm output timer config(s) */
	if (!valid_pwm_timer_config())
		return PWM_ESC_ERROR_FATAL;

	/* Init PWM timer config variable(s) */
	PWM_OUT_TIMxClkRefFreqMHz = Get_TIMxClkRefFreqMHz(phtim_esc1); // can use any timer handle after validating their config
	if (PWM_OUT_TIMxClkRefFreqMHz == 0)
		return PWM_ESC_ERROR_FATAL;

	/* Init esc command min/max */
	*esc_cmd_min = PWM_OUT_TIMxClkRefFreqMHz * PWM_PULSE_PROTO_MIN_US;	// (3000 => 1ms pulse @ 50Hz => 5% duty cycle)
	*esc_cmd_max = PWM_OUT_TIMxClkRefFreqMHz * PWM_PULSE_PROTO_MAX_US;	// (6000 => 2ms pulse @ 50Hz => 10% duty cycle)

	return PWM_ESC_OK;
}

/**
  * @brief deinit pwm protocol config properties
  *
  * @retval pwm esc status
  */
static pwm_esc_status_t pwm_esc_deinit(void) {
	/* Reset PWM out timer handle pointers */
	phtim_esc1 = NULL;
	phtim_esc2 = NULL;
	phtim_esc3 = NULL;
	phtim_esc4 = NULL;

	/* Reset cached PWM timer config variable(s) */
	PWM_OUT_TIMxClkRefFreqMHz = 0U;

	return PWM_ESC_OK;
}

/**
  * @brief starts pwm communication with esc
  *
  * @param	esc_cmd_min		minimum esc command
  * @retval pwm esc status
  */
static pwm_esc_status_t pwm_esc_start(uint32_t esc_cmd_min) {
	/* Set Minimum Duty Cycle */
	SET_DUTY_CYCLE(phtim_esc1, ESC1_PWM_OUT_TIM_CHANNEL, esc_cmd_min);
	SET_DUTY_CYCLE(phtim_esc2, ESC2_PWM_OUT_TIM_CHANNEL, esc_cmd_min);
	SET_DUTY_CYCLE(phtim_esc3, ESC3_PWM_OUT_TIM_CHANNEL, esc_cmd_min);
	SET_DUTY_CYCLE(phtim_esc4, ESC4_PWM_OUT_TIM_CHANNEL, esc_cmd_min);

	/* Init PWM Output Signals */
	if (PWM_Start_Channel(phtim_esc1, ESC1_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_esc2, ESC2_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_esc3, ESC3_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_esc4, ESC4_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	return PWM_ESC_OK;
}


/**
  * @brief stops pwm communication with esc
  *
  * @retval pwm esc status
  */
static pwm_esc_status_t pwm_esc_stop(void) {
	/* De-Init PWM Output Signals */
	if (PWM_Stop_Channel(phtim_esc1, ESC1_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_esc2, ESC2_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_esc3, ESC3_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_esc4, ESC4_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	return PWM_ESC_OK;
}

/**
  * @brief arms esc
  *
  * @param	esc_cmd_idle	idle esc command
  * @retval None
  */
static void pwm_esc_arm(uint32_t esc_cmd_idle) {
	/* Enable Motors (Set Low Duty Cycle) */
	SET_DUTY_CYCLE(phtim_esc1, ESC1_PWM_OUT_TIM_CHANNEL, esc_cmd_idle);
	SET_DUTY_CYCLE(phtim_esc2, ESC2_PWM_OUT_TIM_CHANNEL, esc_cmd_idle);
	SET_DUTY_CYCLE(phtim_esc3, ESC3_PWM_OUT_TIM_CHANNEL, esc_cmd_idle);
	SET_DUTY_CYCLE(phtim_esc4, ESC4_PWM_OUT_TIM_CHANNEL, esc_cmd_idle);
}

/**
  * @brief disarms esc
  *
  * @param	esc_cmd_min		minimum esc command
  * @retval None
  */
static void pwm_esc_disarm(uint32_t esc_cmd_min) {
	/* Disable Motors (Set Minimum Duty Cycle) */
	SET_DUTY_CYCLE(phtim_esc1, ESC1_PWM_OUT_TIM_CHANNEL, esc_cmd_min);
	SET_DUTY_CYCLE(phtim_esc2, ESC2_PWM_OUT_TIM_CHANNEL, esc_cmd_min);
	SET_DUTY_CYCLE(phtim_esc3, ESC3_PWM_OUT_TIM_CHANNEL, esc_cmd_min);
	SET_DUTY_CYCLE(phtim_esc4, ESC4_PWM_OUT_TIM_CHANNEL, esc_cmd_min);
}

/**
  * @brief set duty cycle for pwm signals to ESCs
  *
  * @param  cmd		pointer to esc commands handle
  * @retval pwm esc status
  */
static pwm_esc_status_t pwm_esc_set_commands(const esc_cmds_t *cmd) {
	/* Set Duty Cycle */ // (NOTE: can adjust CCR directly for speed)
	SET_DUTY_CYCLE(phtim_esc1, ESC1_PWM_OUT_TIM_CHANNEL, cmd->esc1);
	SET_DUTY_CYCLE(phtim_esc2, ESC2_PWM_OUT_TIM_CHANNEL, cmd->esc2);
	SET_DUTY_CYCLE(phtim_esc3, ESC3_PWM_OUT_TIM_CHANNEL, cmd->esc3);
	SET_DUTY_CYCLE(phtim_esc4, ESC4_PWM_OUT_TIM_CHANNEL, cmd->esc4);

	return PWM_ESC_OK;
}

/**
  * @brief pwm esc driver initialization
  */
const esc_protocol_interface_t pwm_esc_driver = {
	.init = pwm_esc_init,
	.deinit = pwm_esc_deinit,
	.start = pwm_esc_start,
	.stop = pwm_esc_stop,
	.arm = pwm_esc_arm,
    .disarm = pwm_esc_disarm,
	.set_commands = pwm_esc_set_commands
};
