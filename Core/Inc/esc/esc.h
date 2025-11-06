/*
 * esc.h
 *
 *  Created on: Oct 14, 2025
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef struct {
	float mtr1;
	float mtr2;
	float mtr3;
	float mtr4;
} mtr_cmds_t;

typedef struct {
	uint32_t esc1;
	uint32_t esc2;
	uint32_t esc3;
	uint32_t esc4;
} esc_cmds_t;

typedef enum {
    ESC_OK			= 0x00U,
    ESC_ERROR_WARN	= 0x01U,
	ESC_ERROR_FATAL	= 0x02U
} esc_status_t;

typedef struct {
	esc_status_t (*init)(uint32_t*, uint32_t*);
	esc_status_t (*deinit)(void);
	esc_status_t (*start)(uint32_t);
	esc_status_t (*stop)(void);
    void (*arm)(uint32_t);
    void (*disarm)(uint32_t);
    esc_status_t (*set_commands)(const esc_cmds_t*);
} esc_protocol_interface_t;

typedef struct {
	uint32_t min;
	uint32_t max;
	uint32_t idle;
	uint32_t limit;
	uint32_t liftoff;
} esc_cmd_props_t;

/* Exported function prototypes ----------------------------------------------*/
esc_status_t esc_init(void);

esc_status_t esc_deinit(void);

esc_status_t esc_start(void);

esc_status_t esc_stop(void);

void esc_arm(void);

void esc_disarm(void);

bool esc_is_armed(void);

esc_status_t esc_set_motor_commands(const mtr_cmds_t *mcmd);

void esc_get_command_properties(esc_cmd_props_t *out);
