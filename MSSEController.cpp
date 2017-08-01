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

#include "MSSEController.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

MSSEController::MSSEController()
  : parent() {
}

void MSSEController::usage() {
}

bool MSSEController::parseArg(int& argc, char **& argv) {
  return parent::parseArg(argc, argv);
}

double MSSEController::calcBandwidth(double eps) {
  /* min E[e(k+1)^2]				*/
  /* B = E[C^2] / { E[C] * [ T - s(e(k)) ] }	*/
  double c_mean = getTaskExpValue();
  double c_dev = getTaskDev();
  double period = getTask()->getPeriod();
  double f = c_dev*c_dev / c_mean + c_mean;
  double b_k;
  if (eps < period - f / bw_max) {
    if (eps > 0)
      b_k = f / (period - eps);
    else
      b_k = f / period;
  } else
    b_k = bw_max;
  // Logger::debugLog("# T=%g  e=%g  mc=%g  b=%g\n", period, eps, c_mean, 1.0/u_k);

  return b_k;
}
