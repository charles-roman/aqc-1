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
#include "common/settings.h"
#include "common/hardware.h"

/**
  * @brief  PWM Config Settings
  */
#define PWM_PULSE_MIN_US 		CONFIG_PWM_PULSE_MIN_US
#define PWM_PULSE_MAX_US 		CONFIG_PWM_PULSE_MAX_US
#define PWM_PULSE_PROTO_MIN_US 	CONFIG_PWM_PULSE_PROTO_MIN_US
#define PWM_PULSE_PROTO_MAX_US 	CONFIG_PWM_PULSE_PROTO_MAX_US
#define PWM_PULSE_VALID_MIN_US 	CONFIG_PWM_PULSE_VALID_MIN_US
#define PWM_PULSE_VALID_MAX_US	CONFIG_PWM_PULSE_VALID_MAX_US

/**
  * @brief  Motor Command Settings
  */
#define MTR_CMD_IDLE_PCT		CONFIG_MTR_CMD_IDLE_PCT
#define MTR_CMD_LIFTOFF_PCT		CONFIG_MTR_CMD_LIFTOFF_PCT
#define MTR_CMD_LIMIT_PCT		CONFIG_MTR_CMD_LIMIT_PCT

/**
  * @brief  ESC Channel Aliases
  */
#define MTR3_PWM_TIM_CHANNEL	TIM_CHANNEL_1	// mc pin d6, esc pin s3
#define MTR4_PWM_TIM_CHANNEL	TIM_CHANNEL_2	// mc pin d5, esc pin s4
#define MTR2_PWM_TIM_CHANNEL	TIM_CHANNEL_3	// mc pin d9, esc pin s2
#define MTR1_PWM_TIM_CHANNEL	TIM_CHANNEL_4	// mc pin d10, esc pin s1

/**
  * @brief  ESC Status Type Aliases
  */
#define PWM_ESC_OK				ESC_OK
#define PWM_ESC_ERROR_WARN		ESC_ERROR_WARN
#define PWM_ESC_ERROR_FATAL		ESC_ERROR_FATAL
typedef esc_status_t pwm_esc_status_t;

/**
  * @brief  Timer Handle Pointers
  * 		NOTE: Adjust based on PWM Output Timer Config!
  */
static const TIM_HandleTypeDef phtim_mtr1 = &htim4;
static const TIM_HandleTypeDef phtim_mtr2 = &htim4;
static const TIM_HandleTypeDef phtim_mtr3 = &htim8;
static const TIM_HandleTypeDef phtim_mtr4 = &htim8;

/**
  * @brief  PWM Output Timer Clock Reference Freq
  */
static uint32_t PWM_TIMxClkRefFreqHz;

/**
  * @brief  Motor Command Properties
  */
static float MTR_CMD_MIN;
static float MTR_CMD_MAX;
static float MTR_CMD_IDLE;
static float MTR_CMD_LIFTOFF;
static float MTR_CMD_LIMIT;

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
static inline float MTR_CMD(float pct) {
	return (MTR_CMD_MAX - MTR_CMD_MIN) * (pct / 100.0f) + MTR_CMD_MIN;
}

/**
  * @brief helper function to check equality across four diff uint32_t config setting values
  *
  * @param  a	value a
  * @param  b	value b
  * @param  c	value c
  * @param  d	value d
  *
  * @retval boolean
  */
static inline bool all_equal(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
	return (a == b) && (b == c) && (c == d);
}

/**
  * @brief helper function to validate/constrain motor command
  *
  * @param  cmd		pointer to motor command
  * @retval pwm esc status
  */
static inline pwm_esc_status_t sanitize_pwm_motor_command(float *cmd) {
	if (!inrangef(*cmd, MTR_CMD_MIN, MTR_CMD_MAX)) {
		*cmd = constrainf(*cmd, MTR_CMD_IDLE, MTR_CMD_LIMIT); // constrain within mtr cmd range
		return PWM_ESC_ERROR_WARN;
	}
	return PWM_ESC_OK;
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

	if (!all_equal(MTR1_PWM_TIMClkRefFreqHz, \
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

	if (!all_equal(MTR1_PWM_TIM_CLK_ARR, \
				   MTR2_PWM_TIM_CLK_ARR, \
				   MTR3_PWM_TIM_CLK_ARR, \
				   MTR4_PWM_TIM_CLK_ARR)) {
		return false;
	}

	// NOTE: Might also want to validate polarity, mode, etc.

	/* Validate pwm output freq */
	PWM_OUT_FREQ_HZ = (float) MTR1_PWM_TIMClkRefFreqHz / MTR1_PWM_TIM_CLK_ARR;

	if (PWM_OUT_FREQ_HZ > US_INTERVAL_TO_HZ(PWM_PULSE_VALID_MAX_US)) {
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
	/* Validate pwm output timer config(s) */
	if (!valid_pwm_timer_config())
		return PWM_ESC_ERROR_FATAL;

	/* Init PWM timer config variable(s) */
	PWM_TIMxClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_mtr1); // can use any timer handle after validating their config
	if (PWM_TIMxClkRefFreqHz == 0)
		return PWM_ESC_ERROR_FATAL;

	/* Init motor command properties */
	MTR_CMD_MIN = (USEC_TO_SEC(PWM_PULSE_PROTO_MIN_US) * PWM_TIMxClkRefFreqHz);	// (3000 => 1ms pulse @ 50Hz => 5% duty cycle)
	MTR_CMD_MAX = (USEC_TO_SEC(PWM_PULSE_PROTO_MAX_US) * PWM_TIMxClkRefFreqHz);	// (6000 => 2ms pulse @ 50Hz => 10% duty cycle)

	MTR_CMD_IDLE = MTR_CMD(MTR_CMD_IDLE_PCT);
	if (!inrangef(MTR_CMD_IDLE, MTR_CMD_MIN, MTR_CMD_MAX))
		return PWM_ESC_ERROR_FATAL;

	MTR_CMD_LIFTOFF = MTR_CMD(MTR_CMD_LIFTOFF_PCT);
	if (!inrangef(MTR_CMD_LIFTOFF, MTR_CMD_MIN, MTR_CMD_MAX))
		return PWM_ESC_ERROR_FATAL;

	MTR_CMD_LIMIT = MTR_CMD(MTR_CMD_LIMIT_PCT);
	if (!inrangef(MTR_CMD_LIMIT, MTR_CMD_MIN, MTR_CMD_MAX))
		return PWM_ESC_ERROR_FATAL;

	return PWM_ESC_OK;
}

/**
  * @brief deinit pwm protocol config properties
  *
  * @retval None
  */
static pwm_esc_status_t pwm_esc_deinit(void) {
	/* Reset cached PWM timer config variable(s) */
	PWM_TIMxClkRefFreqHz = 0UL;
	/* Reset cached motor command properties */
	MTR_CMD_MIN = 0.0f;
	MTR_CMD_MAX = 0.0f;
	MTR_CMD_IDLE = 0.0f;
	MTR_CMD_LIFTOFF = 0.0f;
	MTR_CMD_LIMIT = 0.0f;

	return PWM_ESC_OK;
}

/**
  * @brief starts pwm communication with esc
  *
  * @retval None
  */
static pwm_esc_status_t pwm_esc_start(void) {
	/* Set Minimum Duty Cycle */
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);

	/* Init PWM Output Signals */
	if (PWM_Start_Channel(phtim_mtr1, MTR1_PWM_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_mtr2, MTR2_PWM_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_mtr3, MTR3_PWM_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Start_Channel(phtim_mtr4, MTR4_PWM_TIM_CHANNEL) != HAL_OK)
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
	if (PWM_Stop_Channel(phtim_mtr1, MTR1_PWM_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_mtr2, MTR2_PWM_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_mtr3, MTR3_PWM_TIM_CHANNEL) != HAL_OK)
		return PWM_ESC_ERROR_FATAL;

	if (PWM_Stop_Channel(phtim_mtr4, MTR4_PWM_TIM_CHANNEL) != HAL_OK)
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
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_IDLE);
}

/**
  * @brief disarms esc
  *
  * @retval None
  */
static void pwm_esc_disarm(void) {
	/* Disable Motors (Set Minimum Duty Cycle) */
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
}

/**
  * @brief set duty cycle for pwm signals to ESCs
  *
  * @param  cmd		pointer to mtrCommands struct
  * @retval None
  */
static pwm_esc_status_t pwm_esc_set_motor_commands(mtr_cmds_t *cmd) {
	pwm_esc_status_t status = PWM_ESC_OK;
	float mtr1_cmd = cmd->mtr1;
	float mtr2_cmd = cmd->mtr2;
	float mtr3_cmd = cmd->mtr3;
	float mtr4_cmd = cmd->mtr4;

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
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_TIM_CHANNEL, (uint32_t) mtr1_cmd);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_TIM_CHANNEL, (uint32_t) mtr2_cmd);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_TIM_CHANNEL, (uint32_t) mtr3_cmd);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_TIM_CHANNEL, (uint32_t) mtr4_cmd);

	return status;
}

/**
  * @brief pwm_driver initialization
  */
const esc_protocol_interface_t pwm_driver = {
	.init = pwm_esc_init,
	.deinit = pwm_esc_deinit,
	.start = pwm_esc_start,
	.stop = pwm_esc_stop,
	.arm = pwm_esc_arm,
    .disarm = pwm_esc_disarm,
	.set_motor_commands = pwm_esc_set_motor_commands
};
