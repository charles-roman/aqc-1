/*
 * setup.c
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#include "main.h"
#include "setup.h"
#include "../flight/failsafe.h"
#include "../flight/attitude.h"
#include "../flight/mixer.h"
#include "../flight/system.h"
#include "../common/maths.h"
#include "../common/led.h"
#include "../common/time.h"
#include "../sensors/mag.h"

/**
  * @brief enable/disable rc communication
  *
  * @param  enable	logical value (1 or 0)
  * @retval None
  */
void rcInput(uint8_t enable)
{
	if (enable)
	{
		/* Init Input Capture Interrupts */
		HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);	//pin A2, rx ch2, pitch request
		HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);	//pin A3, rx ch1, roll request
		HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);	//pin TX, rx ch3, throttle request
		HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);	//pin RX, rx ch4, yaw request
	}
	else if (!enable)
	{
		/* De-Init Input Capture Interrupts */
		HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
		HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
		HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_3);
		HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_4);
	}
}

/**
  * @brief enable/disable esc communication
  *
  * @param  enable	logical value (1 or 0)
  * @retval None
  */
void escInput(uint8_t enable)
{
	if (enable)
	{
		/* Init PWM Output Signals */
		HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);	//pin D6, mtr3
		HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);	//pin D5, mtr4
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);	//pin D9, mtr2
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);	//pin D10, mtr1

		/* Set Minimum Duty Cycle */
		__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, MTR_CMD_MIN);
		__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, MTR_CMD_MIN);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, MTR_CMD_MIN);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, MTR_CMD_MIN); // for esc startup recognition (& safety when enabling esc)
	}
	else if (!enable)
	{
		/* De-Init PWM Output Signals */
		HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
	}
}

/**
  * @brief arm/disarm drone
  *
  * @param  arm		logical value (1 or 0)
  * @retval None
  */
void arm_drone(uint8_t arm)
{
	if (arm)
	{
		/* Enable Motors (Set Low Duty Cycle) */
		__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, MTR_CMD_IDLE);
		__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, MTR_CMD_IDLE);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, MTR_CMD_IDLE);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, MTR_CMD_IDLE);
	}
	else if (!arm)
	{
		/* Disable Motors (Set Minimum Duty Cycle) */
		__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, MTR_CMD_MIN);
		__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, MTR_CMD_MIN);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, MTR_CMD_MIN);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, MTR_CMD_MIN);
	}
}

/**
  * @brief flight ready checks
  *
  * @param  imu		pointer to (imu) device struct
  * @param  st		pointer to systemState struct
  *
  * @retval 		logical value (1 or 0)
  */
uint8_t ready_to_fly(device *imu, systemState *st)
{
	uint16_t count = 0;
	float et;

	do {
		/* Update IMU */
		read_imu_data(imu);

		/* Estimate Attitude */
		get_attitude(imu, st);

		/* Check Quad is Consistently in a Proper Orientation and Throttle is Idle */
		if (right_side_up(imu) && attitude_within_boundaries(st) && throttle_idle(st))
			count++;
		else
			count = 0;

		/* Get Elapsed Time in Seconds */
		et = USEC_TO_SEC((float)get_timestamp());

		/* Toggle LED */
		blink_led(10);

	} while((count < 1000) && (et < 30));

	/* Check if ready to fly */
	if ((count > 1000) || (et < 30))
		return 1;
	else
		return 0;
}

/**
  * @brief call sensor setups
  *
  * @param  sp		pointer to sensorPackage struct
  * @retval None
  */
void sensor_setup(sensorPackage *sp)
{
	lsm6dsox_setup(&sp->imu);
//	lis3mdl_setup(&sp->mag);
//	mtk3333_setup(&sp->gps);
//	bmp390_setup(&sp->bar);
}
