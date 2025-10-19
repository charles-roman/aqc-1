/*
 * esc.h
 *
 *  Created on: Oct 14, 2025
 *      Author: charlieroman
 */

#pragma once

/* Exported types ------------------------------------------------------------*/
typedef struct mtr_cmds {
	float mtr1;
	float mtr2;
	float mtr3;
	float mtr4;
} mtr_cmds_t;

typedef enum {
    ESC_OK			= 0x00U,
    ESC_ERROR_WARN	= 0x01U,
	ESC_ERROR_FATAL	= 0x02U
} esc_status_t;

typedef struct {
	esc_status_t (*init)(void);
	esc_status_t (*deinit)(void);
	esc_status_t (*start)(void);
	esc_status_t (*stop)(void);
    void (*arm)(void);
    void (*disarm)(void);
    esc_status_t (*set_motor_commands)(mtr_cmds_t *);
} esc_protocol_interface_t;

/* Exported function prototypes ----------------------------------------------*/
esc_status_t esc_init(void);

esc_status_t esc_deinit(void);

esc_status_t esc_start(void);

esc_status_t esc_stop(void);

void esc_arm(void);

void esc_disarm(void);

esc_status_t esc_set_motor_commands(mtr_cmds_t *cmd);
