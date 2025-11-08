/*
 * error.h
 *
 *  Created on: Nov 7, 2025
 *      Author: charlieroman
 */

#pragma once

typedef enum {
    STATUS_OK = 0U,
    STATUS_ERROR_WARN,
    STATUS_ERROR_FATAL
} status_t;

/* Exported macro functions -----------------------------------------------------*/
#define CHECK(status) \
    do { \
        if ((status) == STATUS_ERROR_FATAL) { \
            Error_Handler(); \
        } \
    } while (0)
