/*
 * ARSim
 * Copyright (c) 2000-2010 Tommaso Cucinotta
 *
 * This file is part of ARSim.
 *
 * ARSim is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * ARSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with ARSim. If not, see <http://www.gnu.org/licenses/>
 */

/* Interface includes */

#include "PIController.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

PIController::PIController()
  : parent() {
  z1 = DEF_PI_Z1;
  z2 = DEF_PI_Z2;
  eps_target = DEF_PI_EPS_TARGET;
  u_prev = 1.0;
  eps_prev = 0.0;
}

bool PIController::checkParams() {
  return parent::checkParams();
}

/* Calculate scheduler dependent parameters */
void PIController::calcParams() {
  parent::calcParams();
  period = getTask()->getPeriod();
  alpha1 = (1.0 - z1 - z2) / period;
  beta1  = z1 * z2 / period;
  alpha2 = (2.0 - z1 - z2) / period;
  beta2  = (z1 * z2 - 1.0) / period;
  Logger::debugLog("Calculated params: alpha1=%g, beta1=%g, alpha2=%g, beta2=%g\n",
      alpha1, beta1, alpha2, beta2);
}

void PIController::usage() {
  printf("(-s pi)    -z1     First zero of the linearized feedback transfer function\n");
  printf("(-s pi)    -z2     Second zero of the linearized feedback transfer function\n");
  printf("(-s pi)    -et     Target eps(k) value around which system is stabilized\n");
}

bool PIController::parseArg(int& argc, char **& argv) {
  if (parent::parseArg(argc, argv))
    return true;
  if (strcmp(*argv, "-z1") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &z1) == 1, "Expecting double as argument to -z1 option");
  } else if (strcmp(*argv, "-z2") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &z2) == 1, "Expecting double as argument to -z2 option");
  } else if (strcmp(*argv, "-et") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &eps_target) == 1, "Expecting double as argument to -et option");
  } else {
    return false;
  }
  return true;
}

// @note This is the only controller whose feedback law relies
//       on the scheduling error (sched_err), not the start time (eps)
double PIController::calcBandwidth(double sched_err, double eps) {
  // TODO: replace 0.0 with server period, if ceil_model is in use
  double u;
  if (sched_err < 0.0) {
    u = u_prev - alpha1 * (sched_err - eps_target) - beta1 * (eps_prev - eps_target);
  } else {
    u = u_prev - alpha2 * (sched_err - eps_target) - beta2 * (eps_prev - eps_target);
  }

  Logger::debugLog("PI: sched_err=%g, u_prev=%g, u=%g\n", sched_err, u_prev, u);
  double b = 1.0 / u;
  if (b > bw_max) {
    b = bw_max;
    u = 1.0 / b;
  }
  if (b < 0.01) {
    b = 0.01;
    u = 1.0 / b;
  }
  eps_prev = sched_err;
  u_prev = u;
  return b;
}
