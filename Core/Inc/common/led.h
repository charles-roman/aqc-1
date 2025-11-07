/*
 * led.h
 *
 *  Created on: Oct 19, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef enum {
  LED_READY = 0U,
  LED_ERROR = 1U,
  LED_WAITING = 2U
} led_status_t;

typedef enum {
  LED_OFF = 0U,
  LED_ON = !LED_OFF
} led_state_t;

/* Exported functions prototypes ---------------------------------------------*/
void led_on(void);

void led_off(void);

void led_toggle(void);

void led_blink(uint16_t hz);

void led_set_status(led_status_t status);
