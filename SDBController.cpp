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

#include "SDBController.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

SDBController::SDBController()
  : parent() {
  target_eps = 0.0;
}

void SDBController::usage() {
  printf("(-s sdb)   -e      Target s.e. value\n");
}

bool SDBController::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-e") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &target_eps) == 1, "Expecting double as argument to -e option");
  } else {
    return parent::parseArg(argc, argv);
  }
  return true;
}

double SDBController::calcBandwidth(double sched_err, double eps) {
  /* E[e(k+1)] = s(e(k)) + E[C] / B - T = targ
   * B = E[C] / { T + targ - s(e(k)) } */
  double c_mean = getTaskExpValue();
  double b_k;
  double period = getTask()->getPeriod();
  if (eps < period + target_eps - c_mean / bw_max) {
    if (eps > 0)
      b_k = c_mean / (period + target_eps - eps);
    else
      b_k = c_mean / (period + target_eps);
  } else
    b_k = bw_max;
  // Logger::debugLog("# T=%g  e=%g  mc=%g  b=%g\n", period, eps, c_mean, 1.0/u_k);

  return b_k;
}
