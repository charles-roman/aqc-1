/*
 * maths.h
 *
 *  Created on: Oct 13, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
#define PI     3.14159265358979323846f
#define RAD    (PI / 180.0f)

/* Exported macros -----------------------------------------------------------*/
#define DEG_TO_RAD(angle) ((angle) * RAD)
#define RAD_TO_DEG(angle) ((angle) / RAD)
#define MDPS_TO_DPS(rate) ((rate) / 1000)
#define DPS_TO_MDPS(rate) ((rate) * 1000)

#define USEC_TO_SEC(time) ((time) / 1000000)
#define SEC_TO_USEC(time) ((time) * 1000000)
#define HZ_TO_MHZ(freq)   ((freq) / 1000000)
#define MHZ_TO_HZ(freq)   ((freq) * 1000000)
#define HZ_TO_INTERVAL(x) (1 / (x))
#define INTERVAL_TO_HZ(x) (1 / (x))
#define HZ_TO_INTERVAL_US(x) (1000000 / (x))
#define US_INTERVAL_TO_HZ(x) (1000000 / (x))
#define HZ_TO_RAD_PER_SEC(freq) ((freq) * (2 * PI))

#define sq(x) ((x)*(x))

#define MIN(a,b) \
  __extension__ ({ __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a < _b ? _a : _b; })

#define MAX(a,b) \
  __extension__ ({ __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a > _b ? _a : _b; })

#define ABS(x) \
  __extension__ ({ __typeof__ (x) _x = (x); \
  _x > 0 ? _x : -_x; })

#define SIGN(x) \
  __extension__ ({ __typeof__ (x) _x = (x); \
  (_x > 0) - (_x < 0); })

/* Exported static inline functions ---------------------------------------------*/
static inline float constrainf(float val, float min, float max) {
	if (val < min) {
        return min;
	} else if (val > max) {
		return max;
	} else {
        return val;
	}
}

static inline bool inrangef(float val, float min, float max) {
	return (val >= min) && (val <= max);
}

static inline float avgf(const float arr[], size_t len) {
	float avg, sum = 0.0f;

	for (size_t i = 0; i < len; ++i) {
		sum += arr[i];
	}
	avg = sum / (float)len;

	return avg;
}

static inline int signumf(float val) {
	if (num > 0.0) return 1;
	if (num < 0.0) return -1;
	return 0;
}

static inline float mapf(float val, float in_min, float in_max, float out_min, float out_max) {
	return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline bool all_equalu32(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
	return (a == b) && (b == c) && (c == d);
}
