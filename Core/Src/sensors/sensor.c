/*
 * sensor.c
 *
 *  Created on: Oct 27, 2025
 *      Author: charlieroman
 */

#include "sensors/sensor.h"

/**
  * @brief helper function to validate generic sensor driver initialization
  *
  * @param  driver	pointer to sensor driver
  * @retval boolean
  */
bool valid_sensor_driver(const sensor_interface_t *driver) {
	return (driver &&
			driver->init &&
		    driver->deinit &&
			driver->read);
}

/**
  * @brief low pass filter
  *
void lpf_update(float *curr, float prev, float wc, float dt) {

}
*/
