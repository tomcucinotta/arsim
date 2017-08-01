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

#include "LimitedController.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

void LimitedController::setMethod(CalcBandwidthMethod cbmeth) {
  cbw_method = cbmeth;
}

LimitedController::LimitedController()
  : parent() {
  coeff_pos = DEF_P;
  gamma = DEF_g;
  cbw_method = &LimitedController::calcBandwidthLimitedAsymm;
  strict_checks = false;
}

bool LimitedController::checkParams() {
  if (strict_checks) {
    if (! parent::checkParams())
      return false;
    printf("# T=%g  h=%g  H=%g b_max=%g [-e,E]=[%g,%g]\n",
  	 period, c_min, c_max, bw_max, -eps_min, eps_max);
    BCHECK(bw_max <= 1, "b_max > 1");
    BCHECK(period >= c_max / bw_max, "H / b_max > T");
    BCHECK(eps_min + eps_max * c_min / c_max > period * (1 - c_min / c_max), "e+h/H*E <= T(1-h/H)");

    //double alpha = c_min / c_max;
    //double delta = u_min * c_max / period;
    //double aa = (1 - alpha) / (1 + alpha);
    //BCHECK(gamma > aa, "g<=(1-a)/(1+a)");
    //BCHECK(gamma > delta / (2 - delta), "g<=d/(2-d)");
  }
  return true;
}

/* Calculate single invariant, given coeff_pos */
void LimitedController::calcInvariant(double period, double alpha, double coeff_pos, double& eps_min, double& eps_max) {
  Logger::debugLog("# calcInvariant(): T=%g  alpha=%g  cp=%g\n", period, alpha, coeff_pos);
  double min_sum = period * (1 - alpha);
  Logger::debugLog("# calcInvariant(): min_sum = %g\n", min_sum);
  eps_min = min_sum * (1 - coeff_pos) / (1 - (1 - alpha) * coeff_pos);
  eps_max = eps_min * coeff_pos / (1 - coeff_pos);
  Logger::debugLog("# calcInvariant(): [-e, E] = [%g, %g]\n", -eps_min, eps_max);

  eps_min += DELTA;
  eps_max += DELTA;

//    ASSERT(eps_min + eps_max * alpha >= period * (1 - alpha),
//	    "calcInvariant(): e + a E < T ( 1 - a)");
}

/* Calculate scheduler dependent parameters */
void LimitedController::calcParams() {
  parent::calcParams();
  c_min = getTask()->getMinExecutionTime();
  c_max = getTask()->getMaxExecutionTime();
  period = getTask()->getPeriod();

  calcInvariant(period, c_min / c_max, coeff_pos, eps_min, eps_max);
}

void LimitedController::usage() {
  printf("(-s any)   -cp     Percentage of invariant that remains positive\n");
  printf("(-s re)    -g      Exponential reduction factor\n");
  printf("           -sc     Enable strict check of invariant conditions\n");
}

bool LimitedController::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-cp") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &coeff_pos) == 1, "Expecting double as argument to -cp option");
    CHECK((coeff_pos >= 0) && (coeff_pos <= 1), "Expecting double in [0,1] range to -cp option");
  } else if (strcmp(*argv, "-g") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &gamma) == 1, "Expecting double as argument to -g option");
    CHECK((gamma > 0) && (gamma <= 1), "Expecting double in (0,1) range to -g option");
  } else if (strcmp(*argv, "-sc") == 0) {
    strict_checks = true;
  } else {
    return parent::parseArg(argc, argv);
  }
  return true;
}

double LimitedController::calcBandwidth(double sched_err, double start_err) {
  return (this->*cbw_method)(start_err);
}


/** Controller types ************************************************************/

/* Returns u(k) range that guarantees e(k+1) remains in [-E,+E], given e(k) already in that
 * range;  */
Interval LimitedController::calcUIntervalLimitedSymm(double eps, double c_min, double c_max, double eps_max) {
  double uu;
  double UU;
  if (eps > 0) {
    uu = (period - eps_max - eps) / c_min;
    UU = (period + eps_max - eps) / c_max;
  } else {
    uu = (period - eps_max) / c_min;
    UU = (period + eps_max) / c_max;
  }
  return Interval(uu, UU);
}

//double LimitedController::calcBandwidthLimitedSymm(double eps) {
//  Interval ur = calcUIntervalLimitedSymm(eps, c_min, c_max, eps_max);
//  Interval r = ur.intersect(Interval(u_min, MAXDOUBLE));
//  ASSERT(! r.isEmpty(), "Scheduling impossible");
//  return 2.0 / (r.getMin() + r.getMax());
//}

/* Returns u(k) range that guarantees e(k+1) remains in [-e,+E], given e(k) already in that
 * range;  */
Interval LimitedController::calcUIntervalLimitedAsymm(double eps, double period, double c_min, double c_max, double eps_min, double eps_max) {
  double uu;
  double UU;
  if (eps > 0) {
    uu = (period - eps_min - eps) / c_min;
    UU = (period + eps_max - eps) / c_max;
  } else {
    uu = (period - eps_min) / c_min;
    UU = (period + eps_max) / c_max;
  }
  return Interval(uu, UU);
}

Interval LimitedController::calcBwRange(
  double start_err, double period,
  double c_min, double c_max,
  double eps_min, double eps_max
) {
  double bb;
  double BB;
  BB = c_min / (period - eps_min - start_err);
  bb = c_max / (period + eps_max - start_err);
  CHECK(bb <= BB, "Problem with invariant params");
  return Interval(bb, BB);
}

// Interval LimitedController::calcBwRange(double start_err) {
//   return calcBwRange(start_err, period, c_min, c_max, eps_min, eps_max).intersect(Interval(0, bw_max));
// }

double LimitedController::calcBandwidthLimitedAsymm(double eps) {
  Interval ur = calcUIntervalLimitedAsymm(eps, period, c_min, c_max, eps_min, eps_max);
  Interval r = ur.intersect(Interval(1.0 / bw_max, MAXDOUBLE));
  ASSERT(! r.isEmpty(), "Scheduling impossible");
  return 2.0 / (r.getMin() + r.getMax());
}


/* Returns u(k) range that guarantees e(k+1) remains in [-e(k),+E(k)],
   given e(k) already in that range;  */
double LimitedController::calcBandwidthLimitedAsymm_k(double eps) {
  double h = getTask()->getMinExecutionTime();
  double H = getTask()->getMaxExecutionTime();
  Interval ur = calcUIntervalLimitedAsymm(eps, period, h, H, eps_min, eps_max);
  Interval r = ur.intersect(Interval(1.0 / bw_max, MAXDOUBLE));
  ASSERT(! r.isEmpty(), "Scheduling impossible");
  return 2.0 / (r.getMin() + r.getMax());
}


Interval LimitedController::calcUIntervalYAsymm(double eps) {
  // TODO: This don't know what was doing, here. Should depend on eps
  return Interval(period / c_max, period / c_min);
}

double LimitedController::calcBandwidthLimitedEY(double eps) {
  Interval r_e = calcUIntervalLimitedAsymm(eps, period, c_min, c_max, eps_min, eps_max);
  Interval r_y = calcUIntervalYAsymm(eps);
  Interval r = r_e.intersect(r_y).intersect(Interval(1.0 / bw_max, MAXDOUBLE));
  ASSERT(! r.isEmpty(), "Scheduling impossible");
  return 2.0 / (r.getMin() + r.getMax());
}

Interval LimitedController::calcUIntervalReduced(double eps, double c_min, double c_max, double gamma) {
  double uu;
  double UU;
  double alpha = c_min / c_max;
  double e_neg_max = -period/gamma*(1-alpha)/(1+alpha);
  double e_pos_min = period*(1-alpha)/(1-alpha+gamma*(1+alpha));
  double e_pos_max = period/(1-gamma);
  printf("# %g [%g,%g] [%g,%g]\n", eps, -period, e_neg_max, e_pos_min, e_pos_max);
  ASSERT((-period < e_neg_max) && (e_pos_min < e_pos_max), "Uncorrect ranges");
  if ((e_pos_min < eps) && (eps < e_pos_max)) {
    uu = MAX((period - (1 + gamma) * eps) / c_min, 0);
    UU = (period - (1 - gamma) * eps) / c_max;
  } else if ((-period < eps) && (eps < e_neg_max)) {
    uu = (period + gamma * eps) / c_min;
    UU = (period - gamma * eps) / c_max;
  } else {
    Logger::debugLog("Exited exponentially controlled region\n");
    exit(0);
  }
  return Interval(uu, UU);
}

double LimitedController::calcBandwidthReduced(double eps) {
  Interval r_exp = calcUIntervalReduced(eps, c_min, c_max, gamma);
  ASSERT(! r_exp.isEmpty(), "Scheduling impossible");
  Interval r = r_exp.intersect(Interval(1.0 / bw_max, MAXDOUBLE));
  if (r.isEmpty())
    return bw_max;
  else
    return 1.0 / (r.getMax() - DELTA);
    //    return 2.0 / (r.getMin() + r.getMax());
}

/** End Controller types ********************************************************/
