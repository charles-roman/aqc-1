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

/* Exported types ------------------------------------------------------------*/
typedef struct rc_requests {
	float roll;
	float pitch;
	float yaw;
	float throttle;
} rc_reqs_t;

typedef struct {
    void (*init)(void);
    void (*start)(void);
    void (*stop)(void);
    void (*is_armed)(void);
    void (*get_rc_requests)(mtr_cmds_t *);
} esc_protocol_interface_t;

/* Exported functions prototypes ---------------------------------------------*/
void rx_init(void);

void rx_deinit(void);

bool is_armed(GPIO_TypeDef* ARM_GPIO_PORT, uint16_t ARM_GPIO_PIN);

void get_rc_requests(rc_reqs_t *req);
