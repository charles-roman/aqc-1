/*
 * attitude.c
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#include <math.h>
#include "flight/attitude.h"
#include "flight/mixer.h"
#include "esc/esc.h"
#include "common/maths.h"
#include "common/settings.h"

/**
  * @brief  Attitude Estimation Filter Selection Setting
  */
#define ATTITUDE_FILT						CONFIG_ATTITUDE_FILT

/**
  * @brief  Complementary Filter Settings
  */
#define COMP_FILT_GAIN_XL	 				CONFIG_COMP_FILT_GAIN_XL
#define COMP_FILT_GAIN_GYRO	 				CONFIG_COMP_FILT_GAIN_GYRO

/**
  * @brief  Attitude Angle Take-off Limit Settings
  */
#define ROLL_TAKEOFF_LIMIT_DEG				CONFIG_ROLL_TAKEOFF_LIMIT_DEG
#define PITCH_TAKEOFF_LIMIT_DEG				CONFIG_PITCH_TAKEOFF_LIMIT_DEG

/*
 * @brief Attitude PID Controller Settings
 */
#define ROLL_ANGLE_P_GAIN					CONFIG_ROLL_ANGLE_P_GAIN
#define ROLL_ANGLE_I_GAIN					CONFIG_ROLL_ANGLE_I_GAIN
#define ROLL_ANGLE_D_GAIN					CONFIG_ROLL_ANGLE_D_GAIN
#define ROLL_ANGLE_D_LPF_CUTOFF_FREQ_HZ		CONFIG_ROLL_ANGLE_D_LPF_CUTOFF_FREQ_HZ
#define ROLL_ANGLE_CMD_LIM_DPS				CONFIG_ROLL_ANGLE_CMD_LIM_DPS
#define ROLL_ANGLE_I_CMD_LIM_DPS			CONFIG_ROLL_ANGLE_I_CMD_LIM_DPS

#define PID_CONFIG_ROLL_ANGLE				{ROLL_ANGLE_P_GAIN, \
											 ROLL_ANGLE_I_GAIN, \
											 ROLL_ANGLE_D_GAIN, \
											 ROLL_ANGLE_D_LPF_CUTOFF_FREQ_HZ, \
											 ROLL_ANGLE_CMD_LIM_DPS, \
											 ROLL_ANGLE_I_CMD_LIM_DPS}

#define PITCH_ANGLE_P_GAIN					CONFIG_PITCH_ANGLE_P_GAIN
#define PITCH_ANGLE_I_GAIN					CONFIG_PITCH_ANGLE_I_GAIN
#define PITCH_ANGLE_D_GAIN					CONFIG_PITCH_ANGLE_D_GAIN
#define PITCH_ANGLE_D_LPF_CUTOFF_FREQ_HZ	CONFIG_PITCH_ANGLE_D_LPF_CUTOFF_FREQ_HZ
#define PITCH_ANGLE_CMD_LIM_DPS				CONFIG_PITCH_ANGLE_CMD_LIM_DPS
#define PITCH_ANGLE_I_CMD_LIM_DPS			CONFIG_PITCH_ANGLE_I_CMD_LIM_DPS

#define PID_CONFIG_PITCH_ANGLE				{PITCH_ANGLE_P_GAIN, \
											 PITCH_ANGLE_I_GAIN, \
											 PITCH_ANGLE_D_GAIN, \
											 PITCH_ANGLE_D_LPF_CUTOFF_FREQ_HZ, \
											 PITCH_ANGLE_CMD_LIM_DPS, \
											 PITCH_ANGLE_I_CMD_LIM_DPS}

#define ROLL_RATE_P_GAIN					CONFIG_ROLL_RATE_P_GAIN
#define ROLL_RATE_I_GAIN					CONFIG_ROLL_RATE_I_GAIN
#define ROLL_RATE_D_GAIN					CONFIG_ROLL_RATE_D_GAIN
#define ROLL_RATE_D_LPF_CUTOFF_FREQ_HZ		CONFIG_ROLL_RATE_D_LPF_CUTOFF_FREQ_HZ
#define ROLL_RATE_CMD_LIM_PCT				CONFIG_ROLL_RATE_CMD_LIM_PCT
#define ROLL_RATE_I_CMD_LIM_PCT				CONFIG_ROLL_RATE_I_CMD_LIM_PCT

#define PID_CONFIG_ROLL_RATE				{ROLL_RATE_P_GAIN, \
											 ROLL_RATE_I_GAIN, \
											 ROLL_RATE_D_GAIN, \
											 ROLL_RATE_D_LPF_CUTOFF_FREQ_HZ, \
											 ROLL_RATE_CMD_LIM_PCT, \
											 ROLL_RATE_I_CMD_LIM_PCT}

#define PITCH_RATE_P_GAIN					CONFIG_PITCH_RATE_P_GAIN
#define PITCH_RATE_I_GAIN					CONFIG_PITCH_RATE_I_GAIN
#define PITCH_RATE_D_GAIN					CONFIG_PITCH_RATE_D_GAIN
#define PITCH_RATE_D_LPF_CUTOFF_FREQ_HZ		CONFIG_PITCH_RATE_D_LPF_CUTOFF_FREQ_HZ
#define PITCH_RATE_CMD_LIM_PCT				CONFIG_PITCH_RATE_CMD_LIM_PCT
#define PITCH_RATE_I_CMD_LIM_PCT			CONFIG_PITCH_RATE_I_CMD_LIM_PCT

#define PID_CONFIG_PITCH_RATE				{PITCH_RATE_P_GAIN, \
											 PITCH_RATE_I_GAIN, \
											 PITCH_RATE_D_GAIN, \
											 PITCH_RATE_D_LPF_CUTOFF_FREQ_HZ, \
											 PITCH_RATE_CMD_LIM_PCT, \
											 PITCH_RATE_I_CMD_LIM_PCT}

#define YAW_RATE_P_GAIN						CONFIG_YAW_RATE_P_GAIN
#define YAW_RATE_I_GAIN						CONFIG_YAW_RATE_I_GAIN
#define YAW_RATE_D_GAIN						CONFIG_YAW_RATE_D_GAIN
#define YAW_RATE_D_LPF_CUTOFF_FREQ_HZ		CONFIG_YAW_RATE_D_LPF_CUTOFF_FREQ_HZ
#define YAW_RATE_CMD_LIM_PCT				CONFIG_YAW_RATE_CMD_LIM_PCT
#define YAW_RATE_I_CMD_LIM_PCT				CONFIG_YAW_RATE_I_CMD_LIM_PCT

#define PID_CONFIG_YAW_RATE					{YAW_RATE_P_GAIN, \
											 YAW_RATE_I_GAIN, \
											 YAW_RATE_D_GAIN, \
											 YAW_RATE_D_LPF_CUTOFF_FREQ_HZ, \
											 YAW_RATE_CMD_LIM_PCT, \
											 YAW_RATE_I_CMD_LIM_PCT}

/*
 * @brief ESC Command Settings
 */
#define ESC_CMD_LIFTOFF_PCT					CONFIG_ESC_CMD_LIFTOFF_PCT

/*
 * @brief Attitude PID Controllers
 */
static pid_ctrl_t roll_angle_pid;
static pid_ctrl_t pitch_angle_pid;
static pid_ctrl_t roll_rate_pid;
static pid_ctrl_t pitch_rate_pid;
static pid_ctrl_t yaw_rate_pid;


#if ATTITUDE_FILT == COMP_FILT_ID
/**
  * @brief gets attitude of quad-copter in terms of Euler angles w.r.t body frame via complementary filter
  *
  * @param  imu		read-only pointer to imu 6d sensor handle
  * @param	est		pointer to attitude handle
  *
  * @retval None
  */
static void complementary_filter(const imu_6D_t *imu, attitude_est_t *est) {
	/*
	 * No Inf check needed due to bounded xl data
	 * No NaN check needed due to guaranteed valid xl data
	 */

	/* Convert accel data to roll and pitch angle estimates */
	float xl_roll_est_deg = RAD_TO_DEG(atan2f(imu->accel_y, sqrtf(sq(imu->accel_z) + sq(imu->accel_x))));
	float xl_pitch_est_deg = RAD_TO_DEG(atan2f(-(imu->accel_x), sqrtf(sq(imu->accel_z) + sq(imu->accel_y))));

	/* Convert gyro data to roll and pitch angle estimates */
	float gyro_roll_est_deg = (est->roll_rate_dps * USEC_TO_SEC(imu->dt)) + est->roll_angle_deg;
	float gyro_pitch_est_deg = (est->pitch_rate_dps * USEC_TO_SEC(imu->dt)) + est->pitch_angle_deg;

	/* Apply complementary filter to get combined estimates */
	est->roll_angle_deg = (COMP_FILT_GAIN_GYRO) * gyro_roll_est_deg + (COMP_FILT_GAIN_XL) * xl_roll_est_deg;
	est->pitch_angle_deg = (COMP_FILT_GAIN_GYRO) * gyro_pitch_est_deg + (COMP_FILT_GAIN_XL) * xl_pitch_est_deg;
}
#endif

/**
  * @brief updates angular rates and (conditionally) attitude of quad-copter through configured filter
  *
  * @param  imu		read-only pointer to imu 6d sensor handle
  * @param	est		pointer to attitude handle
  *
  * @retval attitude status type
  */
attitude_status_t attitude_estimator_update(const imu_6D_t *imu, attitude_est_t *est) {
	/* Get Rates */
	est->roll_rate_dps = (MDPS_TO_DPS(imu->rate_x));	// roll rate gets gyro x-axis rate
	est->pitch_rate_dps = (MDPS_TO_DPS(imu->rate_y));	// pitch rate gets gyro y-axis rate
	est->yaw_rate_dps = (MDPS_TO_DPS(imu->rate_z));		// yaw rate gets gyro z-axis rate */

	/* Get Angles */
	#if ATTITUDE_FILT == COMP_FILT_ID
		complementary_filter(imu, est);
	#endif

	return ATTITUDE_OK;
}

/**
  * @brief helper function to resync pid controllers on flight mode switch
  *
  * @param  req			pointer to rc requests handle
  * @param	est			pointer to attitude estimates handle
  * @param	curr_mode	current flight mode
  *
  * @retval None
  */
static void flight_mode_switch_check(const rc_reqs_t *req, const attitude_est_t *est, mode_status_t curr_mode) {
	static mode_status_t prev_mode = ANGLE_MODE;

	if (curr_mode != prev_mode) {
		if (curr_mode == ANGLE_MODE) {
			/* Resync angle PIDs to current attitude on mode switch */
			pid_resync(&roll_angle_pid, req->roll_angle, est->roll_angle_deg);
			pid_resync(&pitch_angle_pid, req->pitch_angle, est->pitch_angle_deg);

		}
		/* RATE mode: no reset needed; rate PIDs continue running */
		prev_mode = curr_mode;
	}
}

/**
  * @brief helper function to freeze/unfreeze PID integrators
  *
  * @param  throttle	current requested throttle value
  * @retval None
  */
static void integrator_hold_check(float throttle) {
	static bool prev_integrator_hold = false;
	bool low_throttle = (throttle < ESC_CMD_LIFTOFF_PCT);
	bool armed = esc_is_armed();
	bool integrator_hold =  low_throttle || !armed;

	/* Detect Change in Integrator Hold State */
	if (integrator_hold != prev_integrator_hold) {
		if (integrator_hold) {
			/* Disable Integrators */
			roll_angle_pid.integrator_enable  = false;
			pitch_angle_pid.integrator_enable = false;
			roll_rate_pid.integrator_enable   = false;
			pitch_rate_pid.integrator_enable  = false;
			yaw_rate_pid.integrator_enable    = false;

			if (!armed) {
				/* Reset Integrators */
				roll_angle_pid.integrator  = 0.0f;
				pitch_angle_pid.integrator = 0.0f;
				roll_rate_pid.integrator   = 0.0f;
				pitch_rate_pid.integrator  = 0.0f;
				yaw_rate_pid.integrator    = 0.0f;

			}
				// optionally decay on low throttle

			/*
			 * decay_rate should be tuned relative to the sample time (dt)
			 * so decay is fast enough to avoid fighting lift-off but slow
			 * enough to avoid abrupt control changes;
			 *
			 * Some stacks even scale decay by throttle distance to lift-off,
			 * creating a more nuanced “soft hold” behavior.
			 */

		} else {
			/* Enable Integrators */
			roll_angle_pid.integrator_enable  = true;
			pitch_angle_pid.integrator_enable = true;
			roll_rate_pid.integrator_enable   = true;
			pitch_rate_pid.integrator_enable  = true;
			yaw_rate_pid.integrator_enable    = true;

		}
		prev_integrator_hold = integrator_hold;
	}
}

/**
  * @brief updates attitude PID controllers
  *
  * @param cmd		pointer to attitude commands handle
  * @param req		pointer to rc requests handle
  * @param est		pointer to attitude estimates handle
  * @param dt		timestep
  *
  * @retval 		attitude status type
  */
attitude_status_t attitude_controller_update(attitude_cmd_t *cmd, const rc_reqs_t *req, const attitude_est_t *est, float dt) {
	mode_status_t flight_mode = rc_get_flight_mode();

	/* Detect Mode Change and Handle Transition */
	flight_mode_switch_check(req, est, flight_mode);

	/* Handle Integrator Hold/Reset */
	integrator_hold_check(req->throttle);

	/* Get Roll/Pitch Rate Requests */
	float roll_rate_req = req->roll_rate;
	float pitch_rate_req = req->pitch_rate;

	/* Update Roll/Pitch Rate Requests (if in Angle Mode) */
	if (flight_mode == ANGLE_MODE) {
		/* Apply Angle PIDs */
		roll_rate_req = pid_update(&roll_angle_pid, req->roll_angle, est->roll_angle_deg, dt);
		pitch_rate_req = pid_update(&pitch_angle_pid, req->pitch_angle, est->pitch_angle_deg, dt);
	}

	/* Apply Rate PIDs */
	cmd->roll = pid_update(&roll_rate_pid, roll_rate_req, est->roll_rate_dps, dt);
	cmd->pitch = pid_update(&pitch_rate_pid, pitch_rate_req, est->pitch_rate_dps, dt);
	cmd->yaw = pid_update(&yaw_rate_pid, req->yaw_rate, est->yaw_rate_dps, dt);

	return ATTITUDE_OK;
}

/**
  * @brief init attitude PID controllers
  *
  * @retval None
  */
void attitude_controller_init(void) {
	/* Init Angle PIDs */
	pid_init(&roll_angle_pid, &(pid_config_t)PID_CONFIG_ROLL_ANGLE);
	pid_init(&pitch_angle_pid, &(pid_config_t)PID_CONFIG_PITCH_ANGLE);

	/* Init Rate PIDs */
	pid_init(&roll_rate_pid, &(pid_config_t)PID_CONFIG_ROLL_RATE);
	roll_rate_pid.limit = map_pct_to_mtr_cmd(roll_rate_pid.limit);
	roll_rate_pid.integrator_limit = map_pct_to_mtr_cmd(roll_rate_pid.integrator_limit);

	pid_init(&pitch_rate_pid, &(pid_config_t)PID_CONFIG_PITCH_RATE);
	pitch_rate_pid.limit = map_pct_to_mtr_cmd(pitch_rate_pid.limit);
	pitch_rate_pid.integrator_limit = map_pct_to_mtr_cmd(pitch_rate_pid.integrator_limit);

	pid_init(&yaw_rate_pid, &(pid_config_t)PID_CONFIG_YAW_RATE);
	yaw_rate_pid.limit = map_pct_to_mtr_cmd(yaw_rate_pid.limit);
	yaw_rate_pid.integrator_limit = map_pct_to_mtr_cmd(yaw_rate_pid.integrator_limit);
}

/**
  * @brief determines if quad-copter is right side up
  *
  * @param  imu		read-only pointer to imu 6d sensor handle
  * @retval	boolean
  */
bool attitude_is_right_side_up(const imu_6D_t *imu) {
	/* Check sign of accel vector z component */
	return SIGN(imu->accel_z) == POSITIVE;
}

/**
  * @brief determines if quad-copter attitude is within limits
  *
  * @param  est		read-only pointer to attitude handle
  * @retval	boolean
  */
bool attitude_within_limits(const attitude_est_t *est) {
	/* Check roll & pitch angles are within tolerance */
	return (fabs(est->roll_angle_deg) < ROLL_TAKEOFF_LIMIT_DEG) && (fabs(est->pitch_angle_deg) < PITCH_TAKEOFF_LIMIT_DEG);
}
