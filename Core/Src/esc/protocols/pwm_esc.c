/*
 * pwm_esc.c
 *
 *  Created on: Oct 14, 2025
 *      Author: charlieroman
 */

#include <stdint.h>
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
  */
static inline HAL_StatusTypeDef PWM_Start_Channel(TIM_HandleTypeDef *htim, uint32_t channel) {
	return HAL_TIM_PWM_Start(htim, channel);
}

/**
  * @brief  wraps HAL_TIM_PWM_Stop function
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
  * @brief detect and validate pwm output timer configuration(s)
  *
  * @retval None
  */
static void detect_pwm_timer_config(void) {
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

	if ((MTR1_PWM_TIMClkRefFreqHz ^
		 MTR2_PWM_TIMClkRefFreqHz ^
		 MTR3_PWM_TIMClkRefFreqHz ^
		 MTR4_PWM_TIMClkRefFreqHz) != 0) {
		// Setup_Error_Handler();
	}

	/* Cache pwm timer clock reference freq */
	PWM_TIMxClkRefFreqHz = MTR1_PWM_TIMClkRefFreqHz;

	/* Validate pwm timer clock ARRs */
	MTR1_PWM_TIM_CLK_ARR = phtim_mtr1->Init.Period;
	MTR2_PWM_TIM_CLK_ARR = phtim_mtr2->Init.Period;
	MTR3_PWM_TIM_CLK_ARR = phtim_mtr3->Init.Period;
	MTR4_PWM_TIM_CLK_ARR = phtim_mtr4->Init.Period;

	if ((MTR1_PWM_TIM_CLK_ARR ^
		 MTR2_PWM_TIM_CLK_ARR ^
		 MTR3_PWM_TIM_CLK_ARR ^
		 MTR4_PWM_TIM_CLK_ARR) != 0) {
		// Setup_Error_Handler();
	}

	/* Validate pwm output freq */
	PWM_OUT_FREQ_HZ = (float)PWM_TIMxClkRefFreqHz / MTR1_PWM_TIM_CLK_ARR;

	if (PWM_OUT_FREQ_HZ > US_INTERVAL_TO_HZ(PWM_PULSE_VALID_MAX_US)) {
		// Setup_Error_Handler();
	}

	// NOTE: Might also want to validate polarity, mode, etc.
}

/**
  * @brief init pwm output motor command properties
  * 	   NOTE: must call init_pwm_out_timclk_ref_props() first!
  *
  * @retval None
  */
static void compute_motor_command_props(void) {
	/* Init command limits */
	MTR_CMD_MIN = (USEC_TO_SEC(PWM_PULSE_PROTO_MIN_US) * PWM_TIMxClkRefFreqHz);	// (3000 => 1ms pulse @ 50Hz => 5% duty cycle)
	MTR_CMD_MAX = (USEC_TO_SEC(PWM_PULSE_PROTO_MAX_US) * PWM_TIMxClkRefFreqHz);	// (6000 => 2ms pulse @ 50Hz => 10% duty cycle)

	/* Init command settings for idle, lift-off, and limit */
	MTR_CMD_IDLE = MTR_CMD(MTR_CMD_IDLE_PCT);
	MTR_CMD_LIFTOFF = MTR_CMD(MTR_CMD_LIFTOFF_PCT);
	MTR_CMD_LIMIT = MTR_CMD(MTR_CMD_LIMIT_PCT);
}

/**
  * @brief init pwm output motor command properties
  * 	   NOTE: must call init_pwm_out_timclk_ref_props() first!
  *
  * @retval None
  */
static void init_pwm_comm_protocol(void) {
	/* Detect/Validate pwm output timer configuration(s) */
	detect_pwm_timer_config();
	/* Init pwm motor command properties */
	compute_motor_command_props();
}

/**
  * @brief starts pwm communication with esc
  *
  * @param  None
  * @retval None
  */
static void start_pwm_output(void) {
	/* Set Minimum Duty Cycle */
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);

	/* Init PWM Output Signals */
	if (PWM_Start_Channel(phtim_mtr1, MTR1_PWM_TIM_CHANNEL) != HAL_OK) {
		return; // Setup_Error_Handler();
	}

	if (PWM_Start_Channel(phtim_mtr2, MTR2_PWM_TIM_CHANNEL) != HAL_OK) {
		return; // Setup_Error_Handler();
	}

	if (PWM_Start_Channel(phtim_mtr3, MTR3_PWM_TIM_CHANNEL) != HAL_OK) {
		return; // Setup_Error_Handler();
	}

	if (PWM_Start_Channel(phtim_mtr4, MTR4_PWM_TIM_CHANNEL) != HAL_OK) {
		return; // Setup_Error_Handler();
	}
}


/**
  * @brief stops pwm communication with esc
  *
  * @param  None
  * @retval None
  */
static void stop_pwm_output(void) {
	/* De-Init PWM Output Signals */
	if (PWM_Stop_Channel(phtim_mtr1, MTR1_PWM_TIM_CHANNEL) != HAL_OK) {
		return; // Error_Handler();
	}

	if (PWM_Stop_Channel(phtim_mtr2, MTR2_PWM_TIM_CHANNEL) != HAL_OK) {
		return; // Error_Handler();
	}

	if (PWM_Stop_Channel(phtim_mtr3, MTR3_PWM_TIM_CHANNEL) != HAL_OK) {
		return; // Error_Handler();
	}

	if (PWM_Stop_Channel(phtim_mtr4, MTR4_PWM_TIM_CHANNEL) != HAL_OK) {
		return; // Error_Handler();
	}
}

/**
  * @brief arms drone
  *
  * @param  None
  * @retval None
  */
static void arm_over_pwm(void) {
	/* Enable Motors (Set Low Duty Cycle) */
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_IDLE);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_IDLE);
}

/**
  * @brief disarms drone
  *
  * @param  None
  * @retval None
  */
static void disarm_over_pwm(void) {
	/* Disable Motors (Set Minimum Duty Cycle) */
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_TIM_CHANNEL, (uint32_t) MTR_CMD_MIN);
}

/**
  * @brief validates motor command
  *
  * @param  cmd		motor command
  * @retval validated motor command
  */
static float validate_motor_command(float cmd) {
	float ret;

	if (!inrangef(cmd, MTR_CMD_MIN, MTR_CMD_MAX)) {
		// Runtime_Error_Handler();
		ret = constrainf(cmd, MTR_CMD_IDLE, MTR_CMD_LIMIT); // constrain within mtr cmd range
	}

	return ret;
}

/**
  * @brief set duty cycle for pwm signals to ESCs
  *
  * @param  cmd		pointer to mtrCommands struct
  * @retval None
  */
static void set_motor_commands_over_pwm(mtr_cmds_t *cmd) {
	/* Validate Motor Commands */
	float mtr1_cmd = validate_motor_command(cmd->mtr1);
	float mtr2_cmd = validate_motor_command(cmd->mtr2);
	float mtr3_cmd = validate_motor_command(cmd->mtr3);
	float mtr4_cmd = validate_motor_command(cmd->mtr4);

	/* Set Duty Cycle */ // (NOTE: can adjust CCR directly for speed)
	SET_DUTY_CYCLE(phtim_mtr1, MTR1_PWM_TIM_CHANNEL, (uint32_t) mtr1_cmd);
	SET_DUTY_CYCLE(phtim_mtr2, MTR2_PWM_TIM_CHANNEL, (uint32_t) mtr2_cmd);
	SET_DUTY_CYCLE(phtim_mtr3, MTR3_PWM_TIM_CHANNEL, (uint32_t) mtr3_cmd);
	SET_DUTY_CYCLE(phtim_mtr4, MTR4_PWM_TIM_CHANNEL, (uint32_t) mtr4_cmd);
}

/**
  * @brief pwm_driver initialization
  */
const esc_protocol_interface_t pwm_driver = {
	.init = init_pwm_comm_protocol,
	.start = start_pwm_output,
	.stop = stop_pwm_output,
	.arm = arm_over_pwm,
    .disarm = disarm_over_pwm,
	.set_motor_commands = set_motor_commands_over_pwm
};
