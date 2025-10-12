/*
 * rx.h
 *
 *  Created on: Oct 7, 2025
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
bool is_armed(GPIO_TypeDef* ARM_GPIO_Port, uint16_t ARM_GPIO_Pin);

void init_rx_comm_protocol(void);

void start_rx_comm_capture(void);

void get_rc_request(systemState *st);
