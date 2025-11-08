/*
 * sensor.h
 *
 *  Created on: Oct 27, 2025
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
typedef enum {
	SENSOR_OK,
	SENSOR_ERROR_WARN,
	SENSOR_ERROR_FATAL
} sensor_status_t;

typedef struct {
	sensor_status_t (*init)(void);
	sensor_status_t (*deinit)(void);
	sensor_status_t (*read)(void*);
    // void (*update)(void);
    // void (*calibrate)(void);
} sensor_interface_t;

/* Exported functions -------------------------------------------------------*/
bool valid_sensor_driver(const sensor_interface_t *driver);

// void lpf_update(float *curr, float prev, float wc, float dt);
