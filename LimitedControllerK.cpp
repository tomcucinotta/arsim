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

#include "LimitedControllerK.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"
#include "TriSpikeTask.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

LimitedControllerK::LimitedControllerK()
  : parent() {
//   setTask(Task::getInstance("tp"));
  gamma = DEF_g;
}

bool LimitedControllerK::checkParams() {
  //if (! parent::checkParams())
  //return false;
  printf("# T=%g  h=%g  H=%g b_max=%g [-e,E]=[%g,%g]\n",
	 period, c_min, c_max, bw_max, -eps_min, eps_max);
  BCHECK(bw_max <= 1, "b_max > 1");
  BCHECK(period >= c_max / bw_max, "H / b_max > T");
  //double alpha_min = pt->getInfExecutionTimeIRatio();
  double alpha_min = getAlphaMin();
  BCHECK(eps_min + eps_max * alpha_min > period * (1 - alpha_min), "e+a_min*E <= T(1-a_min)");
  return true;
}

double LimitedControllerK::getAlphaMin() {
  DoubleLimitedTask *pt = dynamic_cast<DoubleLimitedTask *>(getTask());
  double h_i = pt->getMinExecutionTimeI();
  double H_i = pt->getMaxExecutionTimeI();
  return c_min / (c_min + H_i - h_i);
}

/* Calculate scheduler dependent parameters */
void LimitedControllerK::calcParams() {
  parent::calcParams();
  DoubleLimitedTask *p_tsk = dynamic_cast<DoubleLimitedTask *>(getTask());
  ASSERT(p_tsk != 0, "LimitedControllerK with no DL Task");

  //double alpha = p_tsk->getInfExecutionTimeIRatio();
  double alpha = getAlphaMin();
  Logger::debugLog("a=%g (%g/%g), a_min=%g\n", c_min/c_max, c_min, c_max, alpha);
  double min_sum = period * (1 - alpha) + DELTA;
  printf("# min_sum=%g\n", min_sum);
  eps_min = min_sum * (1 - coeff_pos) / (1 - (1 - alpha) * coeff_pos);
  eps_max = eps_min * coeff_pos / (1 - coeff_pos);
  printf("# min+max=%g\n", eps_min+eps_max);
  printf("# e=%g, E=%g, E+e*h/H=%g\n", eps_min, eps_max, eps_min+eps_max*c_min/c_max);
}

void LimitedControllerK::usage() {
}

bool LimitedControllerK::parseArg(int& argc, char **& argv) {
  return parent::parseArg(argc, argv);
}

/* Returns u(k) range that guarantees e(k+1) remains in [-e(k),+E(k)],
   given e(k) already in that range;  */
double LimitedControllerK::calcBandwidth(double sched_err, double eps) {
  double hh, HH;
  DoubleLimitedTask *pt = dynamic_cast<DoubleLimitedTask *>(getTask());
  ASSERT(pt != NULL, "LimitedControllerK with not a DL Task");


  Logger::debugLog("calcBandwidth(): Calling getMinI() and getMaxI()\n");
  hh = pt->getMinExecutionTimeI();
  HH = pt->getMaxExecutionTimeI();

  Logger::debugLog("calcBandwidth(): [hh, HH]=[%g, %g]\n", hh, HH);
  calcInvariant(period, hh/HH, coeff_pos, eps_min, eps_max);
  ASSERT(eps_min + hh / HH * eps_max >= period * (1 - hh / HH), "e + a h/H < T(1-h/H)");

  Interval ur = calcUIntervalLimitedAsymm(eps, period, hh, HH, eps_min, eps_max);
  ASSERT(! ur.isEmpty(), "Scheduling impossible");
  Interval r = ur.intersect(Interval(1.0 / bw_max, MAXDOUBLE));
  //if (/*(eps_prev > eps_max) ||*/ (r.isEmpty()))
  //  return u_min;
  ASSERT(! r.isEmpty(), "Scheduling impossible");
  return 2.0 / (r.getMin() + r.getMax());
}
