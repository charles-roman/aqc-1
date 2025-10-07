/*
 * pwm.c
 *
 *  Created on: Oct 13, 2023
 *      Author: charlieroman
 */

#include "main.h"
#include "pwm.h"
#include "../common/maths.h"
#include "../common/time.h"
#include "../flight/system.h"

//CHANNELS
static icChannels ic = {{.id = TIM_CHANNEL_1},
						{.id = TIM_CHANNEL_2},
						{.id = TIM_CHANNEL_3},
						{.id = TIM_CHANNEL_4}};

/**
  * @brief pwm pulsewidth limit check
  *
  * @param  pw		pulsewidth
  * @retval 		either pulsewidth or inverted pulsewidth
  */
static float check_pulse_width(float pw)
{
	if (pw > PWM_PULSE_MAX)
		return (HZ_TO_INTERVAL_US((float)PWM_FREQ_HZ) - pw); //Invert Pulse Width
	else
		return pw;
}

/**
  * @brief measure pulsewidth of signal
  *
  * @param  htim	pointer to HAL timer struct
  * @param  ch		pointer to channel struct
  *
  * @retval None
  */
static void get_pulse_width(TIM_HandleTypeDef *htim, channel *ch)
{
	uint32_t counter_period, ref_clk_mhz;
	counter_period = htim->Init.Period + 1;
	ref_clk_mhz = APB1_CLK_FREQ_MHZ/(htim->Init.Prescaler + 1);

	if (ch->is_first_captured == 0)
	{
		ch->icval1 = HAL_TIM_ReadCapturedValue(htim, ch->id);
		ch->is_first_captured = 1;
	}

	else
	{
		ch->icval2 = HAL_TIM_ReadCapturedValue(htim, ch->id);

		if (ch->icval2 > ch->icval1)
		{
			ch->icdiff = ch->icval2 - ch->icval1;
		}

		else
		{
			ch->icdiff = (counter_period - ch->icval1) + ch->icval2; //overflow protection
		}

		/* Calculate Pulse Width */
		float pulsewidth_us;
		pulsewidth_us = ((float)ch->icdiff) / (ref_clk_mhz);
		ch->pulsewidth_us = check_pulse_width(pulsewidth_us); //inverse protection

		ch->is_first_captured = 0; //reset edge counter
	}
}

/**
  * @brief call get_pulse_width function for incoming signal
  *
  * @param  htim	pointer to HAL timer struct
  * @retval None
  */
void read_rc_input(TIM_HandleTypeDef *htim)
{
	switch(htim->Channel)
	{
		case HAL_TIM_ACTIVE_CHANNEL_1:
			get_pulse_width(htim, &ic.ch1);
			break;

		case HAL_TIM_ACTIVE_CHANNEL_2:
			get_pulse_width(htim, &ic.ch2);
			break;

		case HAL_TIM_ACTIVE_CHANNEL_3:
			get_pulse_width(htim, &ic.ch3);
			break;

		case HAL_TIM_ACTIVE_CHANNEL_4:
			get_pulse_width(htim, &ic.ch4);
			break;

		default:
			break;
	}
}

/**
  * @brief map pulsewidth of signal to user request in engineering units
  *
  * @param  st		pointer to systemState struct
  * @retval None
  */
void get_user_input(systemState *st)
{
	st->roll.request     = mapf(ic.ch2.pulsewidth_us, PWM_RANGE_MIN, PWM_RANGE_MAX, ROLL_MIN, ROLL_MAX);
	st->pitch.request    = mapf(ic.ch1.pulsewidth_us, PWM_RANGE_MIN, PWM_RANGE_MAX, PITCH_MIN, PITCH_MAX);
	st->throttle.request = mapf(ic.ch3.pulsewidth_us, PWM_RANGE_MIN, PWM_RANGE_MAX, THROTTLE_MIN, THROTTLE_MAX);
	st->yaw.request		 = mapf(ic.ch4.pulsewidth_us, PWM_RANGE_MIN, PWM_RANGE_MAX, YAW_MIN, YAW_MAX);
}

