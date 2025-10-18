/*
 * setup.c
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#include "main.h"
#include "setup/setup.h"
#include "flight/failsafe.h"
#include "flight/attitude.h"
#include "flight/mixer.h"
#include "flight/system.h"
#include "common/maths.h"
#include "common/led.h"
#include "common/time.h"
#include "../sensors/mag.h"

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
