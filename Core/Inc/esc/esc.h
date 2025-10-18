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

typedef struct {
    void (*init)(void);
    void (*start)(void);
    void (*stop)(void);
    void (*arm)(void);
    void (*disarm)(void);
    void (*set_motor_commands)(mtr_cmds_t *);
} esc_protocol_interface_t;

/* Exported functions prototypes ---------------------------------------------*/
void esc_init(void);

void esc_deinit(void);

void arm(void);

void disarm(void);

void set_motor_commands(mtr_cmds_t *cmd);
