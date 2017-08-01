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

#include "OCController.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <math.h>

OCController::OCController()
  : parent() {
  gamma = DEF_OC_GAMMA;
}

void OCController::usage() {
  printf("(-s oc)    -oc-g   Weight of e(k+1) in minimizing E[g*e(k+1)^2 + (1-g)*b]\n");
}

bool OCController::parseArg(int& argc, char **& argv) {
  if (parent::parseArg(argc, argv)) {
    return true;
  } else if (strcmp(*argv, "-oc-g") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &gamma) == 1, "Expecting double as argument to -oc-g option");
    CHECK((gamma >= 0) && (gamma <= 1), "Expecting double in [0,1] range to -oc-g option");
  } else {
    return false;
  }
  return true;
}

double OCController::calcBandwidth(double eps) {
  /* min E[g*e(k+1)^2 + (1-g)*b]		*/
  /* s(x) = (x < 0 ? 0 : x)
   * d(x) = sqrt( g1**2.0 * c2**2.0 + (2.0 / 3.0 * g1 * mc * (1-s(x)))**3.0 )
   * B(x) = (abs(ro+d(x)))**(1.0/3.0) * sgn(ro+d(x)) +(abs(ro-d(x)))**(1.0/3.0) * sgn(ro-d(x))
   */
  double period = getTask()->getPeriod();
  double c_mean = getTaskExpValue();
  double c_dev = getTaskDev();
  if (c_mean == -1)
    c_mean = 20000;
  if (c_dev == -1)
    c_dev = 10000;
  double b_k;
  double g = gamma;

  double mc = c_mean / period;;
  double sc = c_dev / period;

  double c2 = (sc*sc + mc*mc);
  double g1 = g / (1.0 - g);

  double s = (eps < 0 ? 0 : eps / period);
  //Logger::debugLog("# eps=%g  s=%g\n", eps, s);
  double f = g1*g1 * c2*c2 + pow(2.0/3.0 * g1 * mc * (1.0 - s), 3.0);
  if (f < 0) {
    Logger::debugLog("# Warning: Delta negative is not implemented (yet). Assigning B_max\n");
    b_k = bw_max;
  } else {
    double d = sqrt( f );
    double ro = c2 * g1;
    if (ro-d >= 0)
      b_k = pow(ro+d, 1.0/3.0) + pow(ro-d, 1.0/3.0);
    else
      b_k = pow(ro+d, 1.0/3.0) - pow(d-ro, 1.0/3.0);
    //Logger::debugLog("# b_k needs to be %g\n", b_k);
    if (b_k >= bw_max)
      b_k = bw_max;
  }
  //} else
  //  u_k = u_min;
  //Logger::debugLog("# T=%g  e=%g  mc=%g  b=%g\n", period, eps, c_mean, 1.0/u_k);

  return b_k;
}
