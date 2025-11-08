/*
 * rx.c
 *
 *  Created on: Oct 7, 2025
 *      Author: charlieroman
 */

#include <stddef.h>
#include "rx/rx.h"
#include "rx/protocols/pwm_rx.h"
#include "common/settings.h"

/**
  * @brief  Rx Protocol Setting
  */
#define RX_PROTOCOL	CONFIG_RX_PROTOCOL

/**
  * @brief  rx driver pointer for protocol interface
  */
static const rx_protocol_interface_t *rx_driver = NULL;

/**
  * @brief helper function to validate rx_driver initialization
  *
  * @param  driver	pointer to rx driver
  * @retval boolean
  */
static bool valid_rx_driver(const rx_protocol_interface_t *driver) {
	return (driver &&
			driver->init &&
		    driver->deinit &&
			driver->start &&
			driver->stop &&
			driver->get_channel);
}

/**
  * @brief rx API call to init rx protocol driver interface
  *
  * @param  None
  * @retval rx status
  */
rx_status_t rx_init(void) {
	/* NOTE: use pre-processor conditionals based
	* on configured protocol to initialize rx_driver */
	#if RX_PROTOCOL == RX_PWM_PROTOCOL_ID
		rx_driver = &pwm_rx_driver;
	#else
		#error "Invalid Rx Protocol Configuration"
	#endif

	if (!valid_rx_driver(rx_driver))
		return RX_ERROR_FATAL;

	return rx_driver->init();
}

/**
  * @brief rx API call to deinit rx protocol driver interface
  *
  * @param  None
  * @retval rx status
  */
rx_status_t rx_deinit(void) {
	if (!rx_driver)
		return RX_ERROR_WARN;

	rx_driver->deinit();
	rx_driver = NULL;

	return RX_OK;
}

/**
  * @brief rx API call to start comms
  *
  * @param  None
  * @retval rx status
  */
rx_status_t rx_start(void) {
	if (!rx_driver)
		return RX_ERROR_FATAL;

	return rx_driver->start();
}

/**
  * @brief rx API call to stop comms
  *
  * @param  None
  * @retval rx status
  */
rx_status_t rx_stop(void) {
	if (!rx_driver)
		return RX_ERROR_FATAL;

	return rx_driver->stop();
}

/**
  * @brief rx API call to get channel value
  *
  * @param  ch		channel to get value from
  * @retval channel value
  */
uint32_t rx_get_channel(const uint8_t ch) {
	if (!rx_driver)
		return RX_ERROR_FATAL;

	return rx_driver->get_channel(ch);
}
