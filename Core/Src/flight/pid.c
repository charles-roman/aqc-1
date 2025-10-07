/*
 * pid.c
 *
 *  Created on: Oct 16, 2023
 *      Author: charlieroman
 */

#include <stdint.h>
#include <string.h>
#include <math.h>
#include "../common/maths.h"
#include "pid.h"
#include "mixer.h"
#include "system.h"

/**
  * @brief attitude command saturation check
  *
  * @param  st		pointer to state struct
  * @retval 		logical value (1 or 0)
  */
static uint8_t saturation_check(state *st)
{
	return (fabs(st->command) > st->command_limit);
}

/**
  * @brief integral windup check
  *
  * @param  st		pointer to state struct
  * @retval 		logical value (1 or 0)
  */
static uint8_t windup_check(state *st)
{
	return (SIGN(st->error) == SIGN(st->command));
}

/**
  * @brief integral clamping
  *
  * @param  st		pointer to state struct
  * @retval None
  */
static void clamping(state *st)
{
	uint8_t saturating = saturation_check(st);
	uint8_t winding = windup_check(st);
	uint8_t integrating = st->clamp;

	if ((saturating && winding) && integrating)
	{
		st->clamp = 0;
	}
	else if ((!saturating || !winding) && !integrating)
	{
		st->clamp = 1;
	}
}

/**
  * @brief pid controller
  *
  * @param  st		pointer to state struct
  * @param  dt		timestep
  *
  * @retval None
  */
void pid_control(state *st, float dt)
{
	/*PID GAINS*/
	float K[3];
	memcpy(K, st->gains, sizeof(st->gains));

	/*Calculate Error*/
	float error;
	error = st->request - st->estimate;

	/*Calculate Control Signal*/
	float p, i, d;
	p = K[0]*error;
	i = (st->clamp)*(K[1]*dt/2)*(error + st->error) + st->integrator;
	d = ((2.0f*K[2]*LPF_CUTOFF_FREQ)*(st->prev_estimate - st->estimate) +	// (error - st->error)
			(2.0f - dt*LPF_CUTOFF_FREQ)*st->differentiator)/(2.0f + dt*LPF_CUTOFF_FREQ);

	st->command = p + i + d;

	/*Update Error, Control Terms, and Previous Measurement*/
	st->error = error;
	st->integrator = i;
	st->differentiator = d;
	st->prev_estimate = st->estimate;

	/*Inf Check*/

	/*NaN Check*/

	/*Update Integrator Clamp*/
	clamping(st);

	/*Constrain Control Signal*/
	st->command = constrainf(st->command, -st->command_limit, st->command_limit);
}
