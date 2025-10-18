/*
 * pwm.c
 *
 *  Created on: Oct 13, 2023
 *      Author: charlieroman
 */

#include "common/maths.h"
#include "common/time.h"
#include "rx/protocols/pwm_rx.h"

// Input Capture Timer Clock Reference Freq
static float IC_TIMx_CLK_REF_FREQ_MHZ;
static uint16_t IC_TIMx_REF_ARR;

// Pulse Handles
static volatile pulse_t pitch_pulse;
static volatile pulse_t roll_pulse;
static volatile pulse_t throttle_pulse;
static volatile pulse_t yaw_pulse;

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
  * @param  htim	pointer to HAL timer handle
  * @param  channel	timer channel value
  * @param	pul		pointer to pulse handle
  *
  * @retval None
  */
static void update_pulse_edge(TIM_HandleTypeDef *htim, uint32_t channel, pulse_t *pul) {
	// Check if Pulse is Rising or Falling
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
  * @brief Input Capture Callback. ISR triggered when mc detects signal from rx
  *
  * @param  htim	pointer to HAL timer struct
  *
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
			update_pulse_edge(htim, THROTTLE_CHANNEL, &throttle_pulse);

		} else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
			update_pulse_edge(htim, YAW_CHANNEL, &yaw_pulse);

		} else {
			/* Unexpected input capture on unsupported TIM2 channel */
			// Error_Handler(ERROR_CODE);
		}

	} else if (htim->Instance == TIM3) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			update_pulse_edge(htim, PITCH_CHANNEL, &pitch_pulse);

		} else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
			update_pulse_edge(htim, ROLL_CHANNEL, &roll_pulse);

		} else {
			/* Unexpected input capture on unsupported TIM3 channel */
			// Error_Handler(ERROR_CODE);
		}

	} else {
		/* Unexpected input capture on unsupported TIMx */
		// Error_Handler(ERROR_CODE);
	}
}

/**
  * @brief gets latest pulse width of pwm signal
  *
  * @param  pul		pointer to pulse handle
  *
  * @retval None
  */
static float get_pulse_width(pulse_t *pul) {
	uint16_t ic_diff;
	float pulsewidth_us;

	/* Calculate Difference Between Capture Compare Values for Rising and Falling Edges */
	ic_diff = (pul->ic_val_f > pul->ic_val_r) ? pul->ic_val_f - pul->ic_val_r
										 	  : (IC_TIMx_REF_ARR - pul->ic_val_r) + pul->ic_val_f; // overflow protection

	/* Calculate and Return Pulse Width */
	pulsewidth_us = ic_diff / IC_TIMx_CLK_REF_FREQ_MHZ;

	return pulsewidth_us;
}

/**
  * @brief map pulse width of signal to user request in engineering units
  *
  * @param	request buffer to store mapped value
  * @param  pul		pointer to pulse handle
  * @param	min		minimum value of state being mapped to
  * @param	max		maximum value of state being mapped to
  *
  * @retval None
  */
static void map_pulse_to_state_request(float *request, pulse_t *pul, float min, float max) {
	/* Check if Pulse is Updated */
	if (!pul->is_updated) return;

	/* Calculate Pulse Width */
	float pulsewidth_us = get_pulse_width(pul);

	/* Validate Pulse Width */
	if (!inrangef(pulsewidth_us, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US)) {
		/* Handle Invalid Pulse */
		// Error_Handler(ERROR_CODE); --> (failsafe!)
		pulsewidth_us = (pul->id == THROTTLE_PULSE_ID) ? PWM_PULSE_MIN : PWM_PULSE_MED; // set pulse width to neutral state
	}

	/* Map Pulse Width and Update Request */
	*request = mapf(pulsewidth_us, PWM_PULSE_MIN_US, PWM_PULSE_MAX_US, min, max);
	/* Reset Pulse Update Flag */
	pul->is_updated = false;
}

/**
  * @brief update request fields of state handles based on latest rc input
  *
  * @param  st		pointer to system state handle
  *
  * @retval mapped state value
  */
void get_rc_requests_over_pwm(systemState *st) {
	map_pulse_to_state_request(&(st->pitch.request), &pitch_pulse, PITCH_MIN_DEG, PITCH_MAX_DEG);
	map_pulse_to_state_request(&(st->roll.request), &roll_pulse, ROLL_MIN_DEG, ROLL_MAX_DEG);
	map_pulse_to_state_request(&(st->throttle.request), &throttle_pulse, THROTTLE_MIN_PCT, THROTTLE_MAX_PCT);
	map_pulse_to_state_request(&(st->yaw.request), &yaw_pulse, YAW_MIN_DPS, YAW_MAX_DPS);
}

/**
  * @brief initialize input capture timer clock reference properties for calcs
  *
  * @retval None
  */
void init_ic_timclk_ref_props(void) {
	uint32_t TIM2_CLK_REF_FREQ_HZ, TIM3_CLK_REF_FREQ_HZ;
	uint16_t TIM2_ARR, TIM3_ARR;

	// Init input capture timer clock reference freq
	TIM2_CLK_REF_FREQ_HZ = Get_TIM2CLKRefFreqHz();
	TIM3_CLK_REF_FREQ_HZ = Get_TIM3CLKRefFreqHz();

	if (TIM2_CLK_REF_FREQ_HZ != TIM3_CLK_REF_FREQ_HZ) {
		// Setup_Error_Handler();
	}
	IC_TIMx_CLK_REF_FREQ_MHZ = HZ_TO_MHZ((float)TIM2_CLK_REF_FREQ_HZ);

	// Init input capture timer clock reference auto-reload reg
	TIM2_ARR = htim2.Init.Period;
	TIM3_ARR = htim3.Init.Period;

	if (TIM2_ARR != TIM3_ARR) {
		// Setup_Error_Handler();
	}
	IC_TIMx_REF_ARR = TIM2_ARR;
}

/**
  * @brief initialize pwm pulse handles for pulse capture
  *
  * @retval None
  */
void init_pwm_pulse_handles(void) {
	/* All other fields default to false or 0*/

	/* Pitch */
	pitch_pulse.is_rising = true;
	pitch_pulse.id = PITCH_PULSE_ID;
	/* Roll */
	roll_pulse.is_rising = true;
	roll_pulse.id = ROLL_PULSE_ID;
	/* Throttle */
	throttle_pulse.is_rising = true;
	throttle_pulse.id = THROTTLE_PULSE_ID;
	/* Yaw */
	yaw_pulse.is_rising = true;
	yaw_pulse.id = YAW_PULSE_ID;
}

/**
  * @brief enable rx pwm signal capture
  *
  * @retval None
  */
void start_pwm_input_capture(void) {
	/* Init Input Capture Interrupts */
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);	// mcu pinout A2, rx ch2, pitch request
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);	// mcu pinout A3, rx ch1, roll request
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);	// mcu pinout TX, rx ch3, throttle request
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);	// mcu pinout RX, rx ch4, yaw request
}

/**
  * @brief disable rx pwm signal capture
  *
  * @retval None
  */
void stop_pwm_input_capture(void) {
	/* De-Init Input Capture Interrupts */
	HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
	HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_3);
	HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_4);
}
