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

#include "PDNVController.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

PDNVController::PDNVController()
  : parent() {
  target_eps = 0.0;
  spread = 1.0;
}

void PDNVController::usage() {
  printf("(-s pdnv)  -pdnv-e Target s.e. value\n");
  printf("(-s pdnv)  -pdnv-s Spread factor (H_k is multiplied by this)\n");
}

bool PDNVController::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-pdnv-e") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &target_eps) == 1, "Expecting double as argument to -pdnv-e option");
  } else if (strcmp(*argv, "-pdnv-s") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &spread) == 1, "Expecting double as argument to -pdnv-s option");
  } else {
    return parent::parseArg(argc, argv);
  }
  return true;
}

/** Compute the bandwidth needed for guaranteeing a conditional probability of deadline non-violation.
 **
 ** Pr{e(k+1) <= targ_eps} = Pr{s(e(k)) + c_k / B - T <= targ_eps} >= p
 ** If predictor outputs H_k s.t. Pr Pr{C_k <= H_k} >= p,
 ** then: B = H_k / { T + targ_eps - s(e(k)) }
 **
 ** @note
 ** Parameter `p' needs to be supplied to the predictor, not the controller.
 **/
double PDNVController::calcBandwidth(double sched_err, double eps) {
  Interval iv = getTaskExpInterval();
  CHECK(! iv.isEmpty(), "Cannot determine task variability range");
  double H_k = iv.getMax() * spread;
  double b_k;
  double period = getTask()->getPeriod();
  if (eps < period + target_eps - H_k / bw_max) {
    if (eps > 0)
      b_k = H_k / (period + target_eps - eps);
    else
      b_k = H_k / (period + target_eps);
  } else
    b_k = bw_max;
  Logger::debugLog("# T=%g  d=%g  e=%g  H=%g  b=%g\n", period, target_eps, eps, H_k, b_k);

  //bw_prev_iot = b_k;
  bw_prev_iot = H_k / (period);
  if ((bw_prev_iot > bw_max) || (bw_prev_iot <= 0.0001))
    bw_prev_iot = bw_max;

  return b_k;
}
