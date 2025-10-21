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

/* Exported types ------------------------------------------------------------*/
typedef enum {
    RX_OK			= 0x00U,
    RX_ERROR_WARN	= 0x01U,
	RX_ERROR_FATAL	= 0x02U
} rx_status_t;

typedef struct {
	rx_status_t (*init)(void);
	rx_status_t (*deinit)(void);
    rx_status_t (*start)(void);
    rx_status_t (*stop)(void);
    rx_status_t (*get_channel)(uint8_t, uint32_t*);
    // uint32_t (*get_channel_count)(void);
    // bool (*signal_valid)(void);
    // bool (*failsafe_active)(void);
} rx_protocol_interface_t;

/* Exported functions prototypes ---------------------------------------------*/
rx_status_t rx_init(void);

rx_status_t rx_deinit(void);

rx_status_t rx_start(void);

rx_status_t rx_stop(void);

rx_status_t rx_get_channel(uint8_t ch, uint32_t *val);

// uint32_t rx_get_channel_count(void);

// bool rx_signal_valid(void);

// bool rx_failsafe_active(void);
