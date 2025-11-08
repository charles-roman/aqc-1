/*
 * mixer.h
 *
 *  Created on: Oct 18, 2023
 *      Author: charlieroman
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
#include "flight/attitude.h"
#include "esc/esc.h"
#include "common/settings.h"

/* Exported function prototypes ----------------------------------------------*/
void mixer_init(void);

void mixer_update(mtr_cmds_t *mcmd, const attitude_cmd_t *acmd, float throttle_req_pct);

void thrust_compensate(mtr_cmds_t *mcmd, const attitude_est_t *est);

float map_pct_to_mtr_cmd(float pct);
