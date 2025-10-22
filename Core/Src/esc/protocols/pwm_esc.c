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
#define MTR1_PWM_OUT_TIM			TIM4
#define MTR2_PWM_OUT_TIM			TIM4
#define MTR3_PWM_OUT_TIM			TIM8
#define MTR4_PWM_OUT_TIM			TIM8

/**
  * @brief  ESC Channel -> Timer Channel Aliases
  */
#define MTR3_PWM_OUT_TIM_CHANNEL	TIM_CHANNEL_1
#define MTR4_PWM_OUT_TIM_CHANNEL	TIM_CHANNEL_2
#define MTR2_PWM_OUT_TIM_CHANNEL	TIM_CHANNEL_3
#define MTR1_PWM_OUT_TIM_CHANNEL	TIM_CHANNEL_4

/**
  * @brief  Motor Command Settings
  */
#define MTR_CMD_IDLE_PCT			CONFIG_MTR_CMD_IDLE_PCT
#define MTR_CMD_LIFTOFF_PCT			CONFIG_MTR_CMD_LIFTOFF_PCT
#define MTR_CMD_LIMIT_PCT			CONFIG_MTR_CMD_LIMIT_PCT

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
static const TIM_HandleTypeDef* phtim_mtr1 = NULL;
static const TIM_HandleTypeDef* phtim_mtr2 = NULL;
static const TIM_HandleTypeDef* phtim_mtr3 = NULL;
static const TIM_HandleTypeDef* phtim_mtr4 = NULL;

/**
  * @brief  PWM Output Timer Clock Reference Freq
  */
static uint32_t PWM_OUT_TIMxClkRefFreqMHz;

/**
  * @brief  Motor Command Properties
  */
static uint32_t MTR_CMD_MIN;
static uint32_t MTR_CMD_MAX;
static uint32_t MTR_CMD_IDLE;
static uint32_t MTR_CMD_LIFTOFF;
static uint32_t MTR_CMD_LIMIT;

/**
  * @brief  Motor Commands
  */
static uint32_t mtr1_cmd;
static uint32_t mtr2_cmd;
static uint32_t mtr3_cmd;
static uint32_t mtr4_cmd;


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
  * @brief calculates mtr cmd based on pct
  *
  * @param  pct		percentage to set motor command at
  *
  * @retval motor command (pwm duty cycle)
  */
static inline uint32_t MTR_CMD(float pct) {
	return (uint32_t) ((MTR_CMD_MAX - MTR_CMD_MIN) * (pct / 100.0f) + MTR_CMD_MIN);
}

/**
  * @brief helper function to validate/constrain motor command
  *
  * @param  cmd		pointer to motor command
  * @retval pwm esc status
  */
static inline pwm_esc_status_t sanitize_pwm_motor_command(uint32_t *cmd) {
	if (!inrange_u32(*cmd, MTR_CMD_MIN, MTR_CMD_MAX)) {
		*cmd = constrain_u32(*cmd, MTR_CMD_IDLE, MTR_CMD_LIMIT); // constrain within mtr cmd range
		return PWM_ESC_ERROR_WARN;
	}
	return PWM_ESC_OK;
}

/**
  * @brief helper function to get the appropriate timer handle based on hardware config
  *
  * @param  tim		pointer to timer type handle
  * @retval pointer to timer handle type (NULL otherwise)
  */
static TIM_HandleTypeDef* Get_MTR_PWM_OUT_TIM_Handle(TIM_TypeDef* tim) {
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
	uint32_t MTR1_PWM_TIMClkRefFreqHz, \
			 MTR2_PWM_TIMClkRefFreqHz, \
			 MTR3_PWM_TIMClkRefFreqHz, \
			 MTR4_PWM_TIMClkRefFreqHz;

	uint32_t MTR1_PWM_TIM_CLK_ARR, \
			 MTR2_PWM_TIM_CLK_ARR, \
			 MTR3_PWM_TIM_CLK_ARR, \
			 MTR4_PWM_TIM_CLK_ARR;

	float PWM_OUT_FREQ_HZ;

	/* Validate pwm timer clock reference freqs */
	MTR1_PWM_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_mtr1);
	MTR2_PWM_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_mtr2);
	MTR3_PWM_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_mtr3);
	MTR4_PWM_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_mtr4);

	if (!all_equal_u32(MTR1_PWM_TIMClkRefFreqHz, \
				   	   MTR2_PWM_TIMClkRefFreqHz, \
					   MTR3_PWM_TIMClkRefFreqHz, \
					   MTR4_PWM_TIMClkRefFreqHz)) {
		return false;
	}

	/* Validate pwm timer clock ARRs */
	MTR1_PWM_TIM_CLK_ARR = phtim_mtr1->Init.Period;
	MTR2_PWM_TIM_CLK_ARR = phtim_mtr2->Init.Period;
	MTR3_PWM_TIM_CLK_ARR = phtim_mtr3->Init.Period;
	MTR4_PWM_TIM_CLK_ARR = phtim_mtr4->Init.Period;

	if (!all_equal_u32(MTR1_PWM_TIM_CLK_ARR, \
				   	   MTR2_PWM_TIM_CLK_ARR, \
					   MTR3_PWM_TIM_CLK_ARR, \
					   MTR4_PWM_TIM_CLK_ARR)) {
		return false;
	}

	// NOTE: Might also want to validate polarity, mode, etc.

	/* Validate pwm output freq */
	PWM_OUT_FREQ_HZ = (float) MTR1_PWM_TIMClkRefFreqHz / MTR1_PWM_TIM_CLK_ARR;

	if (HZ_TO_INTERVAL_US(PWM_OUT_FREQ_HZ) < (float) PWM_PULSE_VALID_MAX_US) {
		return false;
	}

	return true;
}

/**
  * @brief init pwm protocol config properties
  *
  * @retval None
  */
static pwm_esc_status_t pwm_esc_init(void) {
	phtim_mtr1 = Get_MTR_PWM_OUT_TIM_Handle(MTR1_PWM_OUT_TIM);
	if (phtim_mtr1 == NULL)
		return PWM_ESC_ERROR_FATAL;

	phtim_mtr2 = Get_MTR_PWM_OUT_TIM_Handle(MTR2_PWM_OUT_TIM);
	if (phtim_mtr2 == NULL)
		return PWM_ESC_ERROR_FATAL;

	phtim_mtr3 = Get_MTR_PWM_OUT_TIM_Handle(MTR3_PWM_OUT_TIM);
	if (phtim_mtr3 == NULL)
		return PWM_ESC_ERROR_FATAL;

	phtim_mtr4 = Get_MTR_PWM_OUT_TIM_Handle(MTR4_PWM_OUT_TIM);
	if (phtim_mtr4 == NULL)
		return PWM_ESC_ERROR_FATAL;

	/* Validate pwm output timer config(s) */
	if (!valid_pwm_timer_config())
		return PWM_ESC_ERROR_FATAL;

	/* Init PWM timer config variable(s) */
	PWM_OUT_TIMxClkRefFreqMHz = Get_TIMxClkRefFreqMHz(phtim_mtr1); // can use any timer handle after validating their config
	if (PWM_OUT_TIMxClkRefFreqMHz == 0)
		return PWM_ESC_ERROR_FATAL;

	/* Init motor command properties */
	MTR_CMD_MIN = PWM_OUT_TIMxClkRefFreqMHz * PWM_PULSE_PROTO_MIN_US;	// (3000 => 1ms pulse @ 50Hz => 5% duty cycle)
	MTR_CMD_MAX = PWM_OUT_TIMxClkRefFreqMHz * PWM_PULSE_PROTO_MAX_US;	// (6000 => 2ms pulse @ 50Hz => 10% duty cycle)

	MTR_CMD_IDLE = MTR_CMD(MTR_CMD_IDLE_PCT);
	if (!inrange_u32(MTR_CMD_IDLE, MTR_CMD_MIN, MTR_CMD_MAX))
		return PWM_ESC_ERROR_FATAL;

	MTR_CMD_LIFTOFF = MTR_CMD(MTR_CMD_LIFTOFF_PCT);
	if (!inrange_u32(MTR_CMD_LIFTOFF, MTR_CMD_MIN, MTR_CMD_MAX))
		return PWM_ESC_ERROR_FATAL;

	MTR_CMD_LIMIT = MTR_CMD(MTR_CMD_LIMIT_PCT);
	if (!inrange_u32(MTR_CMD_LIMIT, MTR_CMD_MIN, MTR_CMD_MAX))
		return PWM_ESC_ERROR_FATAL;

	/* Init motor command variables */
	mtr1_cmd = MTR_CMD_MIN;
	mtr2_cmd = MTR_CMD_MIN;
	mtr3_cmd = MTR_CMD_MIN;
	mtr4_cmd = MTR_CMD_MIN;

	return PWM_ESC_OK;
}

/**
  * @brief deinit pwm protocol config properties
  *
  * @retval None
  */
static pwm_esc_status_t pwm_esc_deinit(void) {
	/* Reset PWM out timer handle pointers */
	phtim_mtr1 = NULL;
	phtim_mtr2 = NULL;
	phtim_mtr3 = NULL;
	phtim_mtr4 = NULL;
	/* Reset cached PWM timer config variable(s) */
	PWM_OUT_TIMxClkRefFreqMHz = 0U;
	/* Reset cached motor command properties */
	MTR_CMD_MIN = 0U;
	MTR_CMD_MAX = 0U;
	MTR_CMD_IDLE = 0U;
	MTR_CMD_LIFTOFF = 0U;
	MTR_CMD_LIMIT = 0U;
	/* Set motor command variables to safe state*/
	mtr1_cmd = MTR_CMD_MIN;
	mtr2_cmd = MTR_CMD_MIN;
	mtr3_cmd = MTR_CMD_MIN;
	mtr4_cmd = MTR_CMD_MIN;

	return PWM_ESC_OK;
}

/**
  * @brief starts pwm communication with esc
  *
  * @retval None
  */
static pwm_esc_status_t pwm_esc_start(void) {
	/* Set Minimum Duty Cycle */
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_OUT_TIM_CHANNEL, MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_OUT_TIM_CHANNEL, MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_OUT_TIM_CHANNEL, MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_OUT_TIM_CHANNEL, MTR_CMD_MIN);

	/* Init PWM Output Signals */
	if (PWM_Start_Channel(phtim_mtr1, MTR1_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_mtr2, MTR2_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_mtr3, MTR3_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_mtr4, MTR4_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	return PWM_ESC_OK;
}


/**
  * @brief stops pwm communication with esc
  *
  * @retval None
  */
static pwm_esc_status_t pwm_esc_stop(void) {
	/* De-Init PWM Output Signals */
	if (PWM_Stop_Channel(phtim_mtr1, MTR1_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_mtr2, MTR2_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_mtr3, MTR3_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_mtr4, MTR4_PWM_OUT_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	return PWM_ESC_OK;
}

/**
  * @brief arms esc
  *
  * @retval None
  */
static void pwm_esc_arm(void) {
	/* Enable Motors (Set Low Duty Cycle) */
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_OUT_TIM_CHANNEL, MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_OUT_TIM_CHANNEL, MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_OUT_TIM_CHANNEL, MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_OUT_TIM_CHANNEL, MTR_CMD_IDLE);
}

/**
  * @brief disarms esc
  *
  * @retval None
  */
static void pwm_esc_disarm(void) {
	/* Disable Motors (Set Minimum Duty Cycle) */
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_OUT_TIM_CHANNEL, MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_OUT_TIM_CHANNEL, MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_OUT_TIM_CHANNEL, MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_OUT_TIM_CHANNEL, MTR_CMD_MIN);
}

/**
  * @brief check if esc is armed
  *
  * @param	cmd		pointer to mtr_cmds handle
  * @retval boolean
  */
static bool pwm_esc_is_armed(void) {
	if (mtr1_cmd >= MTR_CMD_IDLE)
		return true;

	if (mtr2_cmd >= MTR_CMD_IDLE)
		return true;

	if (mtr3_cmd >= MTR_CMD_IDLE)
		return true;

	if (mtr4_cmd >= MTR_CMD_IDLE)
		return true;

	return false;
}

/**
  * @brief set duty cycle for pwm signals to ESCs
  *
  * @param  cmd		pointer to mtr_cmds handle
  * @retval None
  */
static pwm_esc_status_t pwm_esc_set_motor_commands(const mtr_cmds_t *cmd) {
	pwm_esc_status_t status = PWM_ESC_OK;
	mtr1_cmd = (uint32_t) cmd->mtr1;
	mtr2_cmd = (uint32_t) cmd->mtr2;
	mtr3_cmd = (uint32_t) cmd->mtr3;
	mtr4_cmd = (uint32_t) cmd->mtr4;

	/* Validate Motor Commands */
	if (sanitize_pwm_motor_command(&mtr1_cmd) != PWM_ESC_OK)
		status = PWM_ESC_ERROR_WARN;

	if (sanitize_pwm_motor_command(&mtr2_cmd) != PWM_ESC_OK)
		status = PWM_ESC_ERROR_WARN;

	if (sanitize_pwm_motor_command(&mtr3_cmd) != PWM_ESC_OK)
		status = PWM_ESC_ERROR_WARN;

	if (sanitize_pwm_motor_command(&mtr4_cmd) != PWM_ESC_OK)
		status = PWM_ESC_ERROR_WARN;

	/* Set Duty Cycle */ // (NOTE: can adjust CCR directly for speed)
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_OUT_TIM_CHANNEL, mtr1_cmd);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_OUT_TIM_CHANNEL, mtr2_cmd);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_OUT_TIM_CHANNEL, mtr3_cmd);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_OUT_TIM_CHANNEL, mtr4_cmd);

	return status;
}

/**
  * @brief pwm_driver initialization
  */
const esc_protocol_interface_t pwm_esc_driver = {
	.init = pwm_esc_init,
	.deinit = pwm_esc_deinit,
	.start = pwm_esc_start,
	.stop = pwm_esc_stop,
	.arm = pwm_esc_arm,
    .disarm = pwm_esc_disarm,
	.is_armed = pwm_esc_is_armed,
	.set_motor_commands = pwm_esc_set_motor_commands
};
