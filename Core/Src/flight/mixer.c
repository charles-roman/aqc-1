/*
 * mixer.c
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#include <math.h>
#include "flight/mixer.h"
#include "common/maths.h"
#include "common/settings.h"

/**
  * @brief Thrust Compensation Config Setting
  */
#define THRUST_COMP			CONFIG_THRUST_COMP

/**
  * @brief Angular Rate Command Limit Config Settings
  */
#define ROLL_CMD_MAX_PCT	CONFIG_ROLL_RATE_CMD_LIM_PCT
#define PITCH_CMD_MAX_PCT	CONFIG_PITCH_RATE_CMD_LIM_PCT
#define YAW_CMD_MAX_PCT		CONFIG_YAW_RATE_CMD_LIM_PCT

/**
  * @brief Throttle Command Limits
  */
static float THROTTLE_CMD_MIN;
static float THROTTLE_CMD_MAX;

/**
  * @brief Motor Command Properties Type
  */
typedef struct {
    float min;
    float max;
    float idle;
    float limit;
    float liftoff;
} mtr_cmd_props_t;

/**
  * @brief Motor Command Properties Handle
  */
static mtr_cmd_props_t mtr_cmd_props;


/**
  * @brief calculates mtr cmd based on pct
  *
  * @param  pct		percentage to set mtr command at
  * @retval mtr command
  */
float map_pct_to_mtr_cmd(float pct) {
	return ((mtr_cmd_props.max - mtr_cmd_props.min) * (pct / 100.0f) + mtr_cmd_props.min);
}

/**
  * @brief init motor mixer
  *
  * @retval None
  */
void mixer_init(void) {
	esc_cmd_props_t esc_cmd_props;
	esc_get_command_properties(&esc_cmd_props);

	/* Init Motor Command Properties from ESC Command Properties */
	mtr_cmd_props.min = (float)esc_cmd_props.min;
	mtr_cmd_props.max = (float)esc_cmd_props.max;
	mtr_cmd_props.idle = (float)esc_cmd_props.idle;
	mtr_cmd_props.liftoff = (float)esc_cmd_props.liftoff;
	mtr_cmd_props.limit = (float)esc_cmd_props.limit;

	/* Compute Max Commands for Roll/Pitch/Yaw */
	float roll_cmd_max = map_pct_to_mtr_cmd(ROLL_CMD_MAX_PCT);
	float pitch_cmd_max = map_pct_to_mtr_cmd(PITCH_CMD_MAX_PCT);
	float yaw_cmd_max = map_pct_to_mtr_cmd(YAW_CMD_MAX_PCT);

	/* Compute and Init Limits for Throttle Commands */
	THROTTLE_CMD_MIN = mtr_cmd_props.idle;
	THROTTLE_CMD_MAX = mtr_cmd_props.limit - (roll_cmd_max + pitch_cmd_max + yaw_cmd_max);
}

/**
  * @brief motor mixing algorithm
  *
  * @param  mcmd				pointer to motor commands handle
  * @param  acmd				read-only pointer to attitude commands handle
  * @param	throttle_req_pct	throttle request (%)
  *
  * @retval None
  */
void mixer_update(mtr_cmds_t *mcmd, const attitude_cmd_t *acmd, float throttle_req_pct) {
	float roll_cmd = acmd->roll;
	float pitch_cmd = acmd->pitch;
	float yaw_cmd = acmd->yaw;

	/* Map Throttle Request Directly to Motor Command */
	float throttle_cmd = mapf(throttle_req_pct, THROTTLE_MIN_PCT, THROTTLE_MAX_PCT, THROTTLE_CMD_MIN, THROTTLE_CMD_MAX);

	/* Mix Em Up Real Nice */
	mcmd->mtr1 = throttle_cmd + roll_cmd - pitch_cmd + yaw_cmd; // FL
	mcmd->mtr2 = throttle_cmd + roll_cmd + pitch_cmd - yaw_cmd; // RL
	mcmd->mtr3 = throttle_cmd - roll_cmd - pitch_cmd - yaw_cmd; // FR
	mcmd->mtr4 = throttle_cmd - roll_cmd + pitch_cmd + yaw_cmd; // RR
}

#if THRUST_COMP == ENABLED
/**
  * @brief thrust compensation algorithm
  *
  * @param  mcmd	pointer to motor commands handle
  * @param  est		read-only pointer to attitude estimates handle
  *
  * @retval None
  */
void thrust_compensate(mtr_cmds_t *mcmd, const attitude_est_t *est) {
	float thrust_ratio = 1.0f / (cosf(DEG_TO_RAD(est->roll_angle_deg)) * cosf(DEG_TO_RAD(est->pitch_angle_deg)));

	mcmd->mtr1 = thrust_ratio * (mcmd->mtr1) - mtr_cmd_props.min * (thrust_ratio - 1);
	mcmd->mtr2 = thrust_ratio * (mcmd->mtr2) - mtr_cmd_props.min * (thrust_ratio - 1);
	mcmd->mtr3 = thrust_ratio * (mcmd->mtr3) - mtr_cmd_props.min * (thrust_ratio - 1);
	mcmd->mtr4 = thrust_ratio * (mcmd->mtr4) - mtr_cmd_props.min * (thrust_ratio - 1);
}
#endif
