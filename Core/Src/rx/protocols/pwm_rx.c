/*
 * pwm.c
 *
 *  Created on: Oct 13, 2023
 *      Author: charlieroman
 */

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "rx/protocols/pwm_rx.h"
#include "common/time.h"
#include "common/maths.h"
#include "common/hardware.h"
#include "common/settings.h"

/**
  * @brief  PWM Config Settings
  */
#define PWM_PULSE_MIN_US    			CONFIG_PWM_PULSE_MIN_US
#define PWM_PULSE_MAX_US    			CONFIG_PWM_PULSE_MAX_US
#define PWM_PULSE_PROTO_MIN_US			CONFIG_PWM_PULSE_PROTO_MIN_US
#define PWM_PULSE_PROTO_MAX_US			CONFIG_PWM_PULSE_PROTO_MAX_US
#define PWM_PULSE_VALID_MIN_US 			CONFIG_PWM_PULSE_VALID_MIN_US
#define PWM_PULSE_VALID_MAX_US			CONFIG_PWM_PULSE_VALID_MAX_US

/**
  * @brief  Rx Channel -> Timer Aliases
  */
#define RX_CH1_IC_TIM					TIM3
#define RX_CH2_IC_TIM					TIM3
#define RX_CH3_IC_TIM					TIM2
#define RX_CH4_IC_TIM					TIM2

/**
  * @brief  Rx Channel -> Timer Channel Aliases
  */
#define RX_CH1_IC_TIM_CHANNEL			TIM_CHANNEL_1
#define RX_CH2_IC_TIM_CHANNEL			TIM_CHANNEL_2
#define RX_CH3_IC_TIM_CHANNEL			TIM_CHANNEL_3
#define RX_CH4_IC_TIM_CHANNEL			TIM_CHANNEL_4

/**
  * @brief  Rx Channel -> Timer Active Channel Aliases
  */
#define RX_CH1_IC_TIM_ACTIVE_CHANNEL	HAL_TIM_ACTIVE_CHANNEL_1
#define RX_CH2_IC_TIM_ACTIVE_CHANNEL	HAL_TIM_ACTIVE_CHANNEL_2
#define RX_CH3_IC_TIM_ACTIVE_CHANNEL	HAL_TIM_ACTIVE_CHANNEL_3
#define RX_CH4_IC_TIM_ACTIVE_CHANNEL	HAL_TIM_ACTIVE_CHANNEL_4

/**
  * @brief  Rx Channel -> GPIO Aliases
  */
#define RX_CH5_GPIO_PORT				GPIOC
#define RX_CH5_GPIO_PIN					GPIO_PIN_2
#define RX_CH6_GPIO_PORT				GPIOC
#define RX_CH6_GPIO_PIN					GPIO_PIN_3

/**
  * @brief  Rx Status Type Aliases
  */
#define PWM_RX_OK						RX_OK
#define PWM_RX_ERROR_WARN				RX_ERROR_WARN
#define PWM_RX_ERROR_FATAL				RX_ERROR_FATAL

typedef rx_status_t pwm_rx_status_t;

/**
  * @brief  Rx Channel Type
  */
typedef enum {
	RX_CH1 = 0x01U,
	RX_CH2 = 0x02U,
	RX_CH3 = 0x03U,
	RX_CH4 = 0x04U,
	RX_CH5 = 0x05U,
	RX_CH6 = 0x06U
} rx_channel_t;

/**
  * @brief  Rx Pulse Handle Type
  */
typedef struct pulse {
	bool is_rising;
	bool is_updated;
	uint32_t ic_val_r;
	uint32_t ic_val_f;
} pulse_t;

/**
  * @brief  Rx Signal Level Type
  */
typedef enum {
	SIGNAL_LOW  = 0U,
	SIGNAL_HIGH = !SIGNAL_LOW
} level_t;

/**
  * @brief  Pulse Handles and Level Variables
  */
static volatile pulse_t rx_ch1_pulse = {.is_rising = true, .is_updated = false};
static volatile pulse_t rx_ch2_pulse = {.is_rising = true, .is_updated = false};
static volatile pulse_t rx_ch3_pulse = {.is_rising = true, .is_updated = false};
static volatile pulse_t rx_ch4_pulse = {.is_rising = true, .is_updated = false};

static volatile level_t rx_ch5_level = SIGNAL_LOW;
static volatile level_t rx_ch6_level = SIGNAL_LOW;

/**
  * @brief  Rx Channel IC Timer Handle Pointers
  * 		NOTE: Adjust based on Input Capture Timer Config(s)!
  */
static const TIM_HandleTypeDef* phtim_rx_ch1 = NULL;
static const TIM_HandleTypeDef* phtim_rx_ch2 = NULL;
static const TIM_HandleTypeDef* phtim_rx_ch3 = NULL;
static const TIM_HandleTypeDef* phtim_rx_ch4 = NULL;

/**
  * @brief  Input Capture Timer Clock Reference Properties
  */
static uint32_t IC_TIMx_REF_ARR;
static uint32_t IC_TIMxClkRefFreqMHz; // NOTE: Keep at 1MHz to avoid unnecessary division calcs \
										  Otherwise this should be a FLOAT to avoid precision loss!
/**
  * @brief  ISR Callback Error Flag
  */
static bool ISR_Callback_Error_Flag;

/**
  * @brief  wraps HAL_TIM_IC_Start_IT function
  *
  * @param  htim	pointer to HAL timer handle
  * @param  channel timer channel
  *
  * @retval HAL status
  */
static inline HAL_StatusTypeDef IC_Start_Channel_IT(TIM_HandleTypeDef *htim, uint32_t channel) {
	return HAL_TIM_IC_Start_IT(htim, channel);
}

/**
  * @brief  wraps HAL_TIM_IC_Stop_IT function
  *
  * @param  htim	pointer to HAL timer handle
  * @param  channel timer channel
  *
  * @retval HAL status
  */
static inline HAL_StatusTypeDef IC_Stop_Channel_IT(TIM_HandleTypeDef *htim, uint32_t channel) {
	 return HAL_TIM_IC_Stop_IT(htim, channel);
}

/**
  * @brief helper function to get the appropriate timer handle based on hardware config
  *
  * @param  tim		pointer to timer type handle
  * @retval pointer to timer handle type (NULL otherwise)
  */
static TIM_HandleTypeDef* Get_Rx_Channel_IC_TIM_Handle(TIM_TypeDef* tim) {
	#if HTIM2 == CONFIGURED
	if (tim == TIM2)
		return &htim2;
	#endif

	#if HTIM3 == CONFIGURED
	if (tim == TIM3)
		return &htim3;
	#endif

	// add more as needed

	return NULL;
}

/**
  * @brief configures input capture for specific timer channel
  *
  * @retval None
  */
static inline void Configure_IC_Polarity(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t polarity) {
    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = polarity;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;

    HAL_TIM_IC_ConfigChannel(htim, &sConfigIC, channel);
}

/**
  * @brief updates pulse handle with value from capture compare register
  *
  * @param  htim		pointer to HAL timer handle
  * @param  channel		timer channel value
  * @param	pul			pointer to pulse handle
  *
  * @retval None
  */
static void update_pulse_edge(TIM_HandleTypeDef *htim, uint32_t channel, pulse_t *pul) {
	/* Check if Pulse is Rising or Falling */
	if (pul->is_rising) {
		pul->ic_val_r = HAL_TIM_ReadCapturedValue(htim, channel);
		Configure_IC_Polarity(htim, channel, TIM_INPUTCHANNELPOLARITY_FALLING);
		pul->is_rising = false; // reset rising edge flag
		pul->is_updated = false; // reset update flag

	} else {
		pul->ic_val_f = HAL_TIM_ReadCapturedValue(htim, channel);
		Configure_IC_Polarity(htim, channel, TIM_INPUTCHANNELPOLARITY_RISING);
		pul->is_rising = true; // set rising edge flag
		pul->is_updated = true; // set update flag
	}
}

/**
  * @brief Input Capture Callback. ISR triggered from pulse edge.
  *
  * @param  htim	pointer to HAL timer struct
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	/* Determine where event occurred */
	if (htim->Instance == RX_CH1_IC_TIM && htim->Channel == RX_CH1_IC_TIM_ACTIVE_CHANNEL) {
		update_pulse_edge(htim, RX_CH1_IC_TIM_CHANNEL, &rx_ch1_pulse);

	} else if (htim->Instance == RX_CH2_IC_TIM && htim->Channel == RX_CH2_IC_TIM_ACTIVE_CHANNEL) {
		update_pulse_edge(htim, RX_CH2_IC_TIM_CHANNEL, &rx_ch2_pulse);

	} else if (htim->Instance == RX_CH3_IC_TIM && htim->Channel == RX_CH3_IC_TIM_ACTIVE_CHANNEL) {
		update_pulse_edge(htim, RX_CH3_IC_TIM_CHANNEL, &rx_ch3_pulse);

	} else if (htim->Instance == RX_CH4_IC_TIM && htim->Channel == RX_CH4_IC_TIM_ACTIVE_CHANNEL) {
		update_pulse_edge(htim, RX_CH4_IC_TIM_CHANNEL, &rx_ch4_pulse);

	} else {
		/* Unexpected input capture on unsupported TIMx/Channel combo */
		ISR_Callback_Error_Flag = true;
		return;
	}
}

/**
  * @brief GPIO EXTI Callback. ISR triggered from pulse edge.
  *
  * @param  GPIO_Pin	GPIO Pin on which interrupt occurred
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	/* Determine where event occurred */
    if (GPIO_Pin == RX_CH5_GPIO_PIN) {
    	/* Cache GPIO value */
    	GPIO_PinState raw = HAL_GPIO_ReadPin(RX_CH5_GPIO_PORT, RX_CH5_GPIO_PIN);
    	rx_ch5_level = (raw == GPIO_PIN_SET) ? SIGNAL_HIGH : SIGNAL_LOW;

    } else if (GPIO_Pin == RX_CH6_GPIO_PIN) {
    	/* Cache GPIO value */
    	GPIO_PinState raw = HAL_GPIO_ReadPin(RX_CH6_GPIO_PORT, RX_CH6_GPIO_PIN);
    	rx_ch6_level = (raw == GPIO_PIN_SET) ? SIGNAL_HIGH : SIGNAL_LOW;

    } else {
		/* Unexpected input on unsupported GPIO */
    	ISR_Callback_Error_Flag = true;
		return;
    }
}

/**
  * @brief helper function to get latest pulse width of pwm signal
  *
  * @param  pul		pointer to pulse handle
  * @param  val		buffer value to store result in
  *
  * @retval None
  */
static void get_pulse_width(pulse_t *pul, uint32_t *val) {
	uint32_t ic_diff, pulsewidth_us;

	/* Check if Pulse is Updated */
	if (!pul->is_updated) {	// Note: if pulse is always "updated," this can cause a silent error
		*val = 0; // 0 signifies no change
		return;
	}

	/* Calculate Difference Between Capture Compare Values for Rising and Falling Edges */
	ic_diff = (pul->ic_val_f > pul->ic_val_r) ? pul->ic_val_f - pul->ic_val_r
											  : (IC_TIMx_REF_ARR - pul->ic_val_r) + pul->ic_val_f; // overflow protection
	/* Calculate Pulse Width */
	pulsewidth_us = ic_diff / IC_TIMxClkRefFreqMHz; // NOTE: this division loses precision unless IC_TIMxClkRefFreqMHz = 1 \
															  or one operand is converted to a floating-point number
	/* Store in buffer */
	*val = pulsewidth_us;

	/* Reset Pulse Update Flag */
	pul->is_updated = false;
}

/**
  * @brief validates input capture timer config(s)
  * 	   NOTE: timer config settings are important to verify if signals are
  * 	   generated from separate timers, otherwise just the counter reload freq
  *
  * @retval boolean
  */
static bool valid_ic_timer_config(void) {
	uint32_t RX_CH1_IC_TIMClkRefFreqHz, \
			 RX_CH2_IC_TIMClkRefFreqHz, \
			 RX_CH3_IC_TIMClkRefFreqHz, \
			 RX_CH4_IC_TIMClkRefFreqHz;

	uint32_t RX_CH1_IC_TIM_CLK_ARR, \
			 RX_CH2_IC_TIM_CLK_ARR, \
			 RX_CH3_IC_TIM_CLK_ARR, \
			 RX_CH4_IC_TIM_CLK_ARR;

	float COUNTER_RELOAD_PERIOD_SEC;

	/* Validate ic timer clock reference freqs */
	RX_CH1_IC_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_rx_ch1);
	RX_CH2_IC_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_rx_ch2);
	RX_CH3_IC_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_rx_ch3);
	RX_CH4_IC_TIMClkRefFreqHz = Get_TIMxClkRefFreqHz(phtim_rx_ch4);

	if (!all_equal_u32(RX_CH1_IC_TIMClkRefFreqHz, \
				   	   RX_CH2_IC_TIMClkRefFreqHz, \
					   RX_CH3_IC_TIMClkRefFreqHz, \
					   RX_CH4_IC_TIMClkRefFreqHz)) {
		return false;
	}

	/* Validate ic timer clock ARRs */
	RX_CH1_IC_TIM_CLK_ARR = phtim_rx_ch1->Init.Period;
	RX_CH2_IC_TIM_CLK_ARR = phtim_rx_ch2->Init.Period;
	RX_CH3_IC_TIM_CLK_ARR = phtim_rx_ch3->Init.Period;
	RX_CH4_IC_TIM_CLK_ARR = phtim_rx_ch4->Init.Period;

	if (!all_equal_u32(RX_CH1_IC_TIM_CLK_ARR, \
				   	   RX_CH2_IC_TIM_CLK_ARR, \
					   RX_CH3_IC_TIM_CLK_ARR, \
					   RX_CH4_IC_TIM_CLK_ARR)) {
		return false;
	}

	// NOTE: Might also want to validate polarity, mode, etc.

	/* Validate reload period */
	COUNTER_RELOAD_PERIOD_SEC = (float) RX_CH1_IC_TIM_CLK_ARR / RX_CH1_IC_TIMClkRefFreqHz; // use any channel

	if (SEC_TO_USEC(COUNTER_RELOAD_PERIOD_SEC) < (float) PWM_PULSE_VALID_MAX_US) {
		return false;
	}

	return true;
}

/**
  * @brief helper function to reset pulse_t type to safe state
  *
  * @retval None
  */
static void reset_pulse(pulse_t *pul) {
	pul->ic_val_f = 0U;
	pul->ic_val_r = 0U;
	pul->is_rising = true;
	pul->is_updated = false;
}

/**
  * @brief init pwm_rx protocol config properties
  *
  * @retval None
  */
static pwm_rx_status_t pwm_rx_init(void) {
	phtim_rx_ch1 = Get_Rx_Channel_IC_TIM_Handle(RX_CH1_IC_TIM);
	if (phtim_rx_ch1 == NULL)
		return PWM_RX_ERROR_FATAL;

	phtim_rx_ch2 = Get_Rx_Channel_IC_TIM_Handle(RX_CH2_IC_TIM);
	if (phtim_rx_ch2 == NULL)
		return PWM_RX_ERROR_FATAL;

	phtim_rx_ch3 = Get_Rx_Channel_IC_TIM_Handle(RX_CH3_IC_TIM);
	if (phtim_rx_ch3 == NULL)
		return PWM_RX_ERROR_FATAL;

	phtim_rx_ch4 = Get_Rx_Channel_IC_TIM_Handle(RX_CH4_IC_TIM);
	if (phtim_rx_ch4 == NULL)
		return PWM_RX_ERROR_FATAL;

	/* Validate IC timer config(s) */
	if (!valid_ic_timer_config())
		return PWM_RX_ERROR_FATAL;

	/* Init IC timer config variable(s) */
	IC_TIMxClkRefFreqMHz = Get_TIMxClkRefFreqMHz(phtim_rx_ch1); // can use any timer handle after validating their config
	if (IC_TIMxClkRefFreqMHz != 1)	// restrict to 1MHz config
		return PWM_RX_ERROR_FATAL;

	IC_TIMx_REF_ARR = phtim_rx_ch1->Init.Period; // can use any timer handle after validating their config

	return PWM_RX_OK;
}

/**
  * @brief deinit pwm_rx protocol config properties
  *
  * @retval None
  */
static pwm_rx_status_t pwm_rx_deinit(void) {
	/* Reset cached IC timer handles */
	phtim_rx_ch1 = NULL;
	phtim_rx_ch2 = NULL;
	phtim_rx_ch3 = NULL;
	phtim_rx_ch4 = NULL;
	/* Reset cached IC timer config variables */
	IC_TIMxClkRefFreqMHz = 0;
	IC_TIMx_REF_ARR = 0;
	/* Reset rx_chx_pulses to safe state */
	reset_pulse(&rx_ch1_pulse);
	reset_pulse(&rx_ch2_pulse);
	reset_pulse(&rx_ch3_pulse);
	reset_pulse(&rx_ch4_pulse);
	/* Reset rx_chx_levels to safe state */
	rx_ch5_level = SIGNAL_LOW;
	rx_ch6_level = SIGNAL_LOW;

	return PWM_RX_OK;
}

/**
  * @brief enable rx pwm signal capture
  *
  * @retval rx status
  */
static pwm_rx_status_t pwm_rx_start(void) {
	/* Start Input Capture Interrupts */
	if (IC_Start_Channel_IT(phtim_rx_ch1, RX_CH1_IC_TIM_CHANNEL) != HAL_OK)
		return PWM_RX_ERROR_FATAL;

	if (IC_Start_Channel_IT(phtim_rx_ch2, RX_CH2_IC_TIM_CHANNEL) != HAL_OK)
		return PWM_RX_ERROR_FATAL;

	if (IC_Start_Channel_IT(phtim_rx_ch3, RX_CH3_IC_TIM_CHANNEL) != HAL_OK)
		return PWM_RX_ERROR_FATAL;

	if (IC_Start_Channel_IT(phtim_rx_ch4, RX_CH4_IC_TIM_CHANNEL) != HAL_OK)
		return PWM_RX_ERROR_FATAL;

	return PWM_RX_OK;
}

/**
  * @brief disable rx pwm signal capture
  *
  * @retval rx status
  */
static pwm_rx_status_t pwm_rx_stop(void) {
	/* Stop Input Capture Interrupts */
	if (IC_Stop_Channel_IT(phtim_rx_ch1, RX_CH1_IC_TIM_CHANNEL) != HAL_OK)
		return PWM_RX_ERROR_FATAL;

	if (IC_Stop_Channel_IT(phtim_rx_ch2, RX_CH2_IC_TIM_CHANNEL) != HAL_OK)
		return PWM_RX_ERROR_FATAL;

	if (IC_Stop_Channel_IT(phtim_rx_ch3, RX_CH3_IC_TIM_CHANNEL) != HAL_OK)
		return PWM_RX_ERROR_FATAL;

	if (IC_Stop_Channel_IT(phtim_rx_ch4, RX_CH4_IC_TIM_CHANNEL) != HAL_OK)
		return PWM_RX_ERROR_FATAL;

	return PWM_RX_OK;
}

/**
  * @brief get pwm channel pulse or logical value
  *
  * @param  ch		channel to get value from
  * @param	val		buffer value to store result in
  *
  * @retval rx status
  */
static pwm_rx_status_t pwm_rx_get_channel(const uint8_t ch, uint32_t *val) {
	pwm_rx_status_t status = PWM_RX_OK;

	switch (ch) {
		case RX_CH1:
			get_pulse_width(&rx_ch1_pulse, val);
			break;

		case RX_CH2:
			get_pulse_width(&rx_ch2_pulse, val);
			break;

		case RX_CH3:
			get_pulse_width(&rx_ch3_pulse, val);
			break;

		case RX_CH4:
			get_pulse_width(&rx_ch4_pulse, val);
			break;

		case RX_CH5:
			*val = (uint32_t) rx_ch5_level;
			break;

		case RX_CH6:
			*val = (uint32_t) rx_ch6_level;
			break;

		default:
			status = PWM_RX_ERROR_WARN;
			break;
	}

	return status;
}

const rx_protocol_interface_t pwm_rx_driver = {
		.init = pwm_rx_init,
		.deinit = pwm_rx_deinit,
		.start = pwm_rx_start,
		.stop = pwm_rx_stop,
		.get_channel = pwm_rx_get_channel,
};
