/*
 * led.h
 *
 *  Created on: Oct 19, 2023
 *      Author: charlieroman
 */

#pragma once

//#ifndef SRC_LED_H_
//#define SRC_LED_H_
//#endif /* SRC_LED_H_ */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  READY = 0U,
  ERROR_ = !READY,
  WAITING = 2U
} LedCode;

typedef enum
{
  OFF = 0U,
  ON = !OFF
} LedStatus;

/* Exported functions prototypes ---------------------------------------------*/
void blink_led(uint16_t hz);

void toggle_led(uint8_t request);

void led_status(uint8_t request);
