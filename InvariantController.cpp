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

#include "InvariantController.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"
#include "ResourceManager.hpp"
#include "LimitedController.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

InvariantController::InvariantController()
  : parent(),
    eps_inv_stat(0.0, 2.0, 1.0),
    rsteps_inv_stat(0.0, 10.0, 1.0),
    range_width_stat(0.0, 1.0, 0.05),
    range_alpha_stat(0.0, 1.0, 0.05)
{
  coeff_pos = DEF_P;
  eps_min = eps_max = UNASSIGNED;
  rsteps_inv = 0;
  strict_checks = false;
}

bool InvariantController::checkParams() {
  if (! parent::checkParams())
    return false;
  if (strict_checks) {
    double period = getTask()->getPeriod();
    printf("# T=%g  h=%g  H=%g hi=%g  Hi=%g b_max=%g [-e,E]=[%g,%g]\n",
  	       period, getTask()->getMinExecutionTime(), getTask()->getMaxExecutionTime(),
           c_min, c_max, bw_max, -eps_min, eps_max);
    BCHECK(bw_max <= 1.0, "bw_max > 1");
    BCHECK(period >= c_max / bw_max, "Hi / b_max > T");
    BCHECK(eps_min + eps_max * c_min / c_max > period * (1 - c_min / c_max), "e+h/H*E <= T(1-h/H)");
  }
  return true;
}

/* Calculate single invariant, given coeff_pos */
void InvariantController::calcInvariant(double period, double alpha, double coeff_pos, double& eps_min, double& eps_max) {
  Logger::debugLog("# calcInvariant(): T=%g  alpha=%g  cp=%g\n", period, alpha, coeff_pos);
  double L = period * (1 - alpha) / (1 - coeff_pos + alpha * coeff_pos);
  eps_min = (1 - coeff_pos) * L;
  eps_max = coeff_pos * L;
  Logger::debugLog("# calcInvariant(): [-e, E] = [%g, %g]\n", -eps_min, eps_max);

  eps_min += DELTA;
  eps_max += DELTA;

  ASSERT(eps_min + eps_max * alpha >= period * (1 - alpha),
	 "calcInvariant(): e + a E < T ( 1 - a)");
}

/* Calculate scheduler dependent parameters */
void InvariantController::calcParams() {
  parent::calcParams();
  Interval iv = getTaskExpInterval();
  CHECK(! iv.isEmpty(), "Cannot determine task variability range");
  c_min = iv.getMin();
  c_max = iv.getMax();

  if ((eps_min == UNASSIGNED) || (eps_max == UNASSIGNED)) {
    double period = getTask()->getPeriod();
    calcInvariant(period, c_min / c_max, coeff_pos, eps_min, eps_max);
  }
}

void InvariantController::usage() {
  printf("(-s any)   -cp     Percentage of invariant that remains positive\n");
  printf("(-s lar)   -e      Invariant lower bound (conflicts with -cp)\n");
  printf("(-s lar)   -E      Invariant upper bound (conflicts with -cp)\n");
}

bool InvariantController::parseArg(int& argc, char **& argv) {
  if (parent::parseArg(argc, argv))
    return true;
  if (strcmp(*argv, "-cp") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &coeff_pos) == 1, "Expecting double as argument to -cp option");
    CHECK((coeff_pos >= 0) && (coeff_pos <= 1), "Expecting double in [0,1] range to -cp option");
  } else if (strcmp(*argv, "-e") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &eps_min) == 1, "Expecting double as argument to -e option");
  } else if (strcmp(*argv, "-E") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &eps_max) == 1, "Expecting double as argument to -E option");
  } else {
    return false;
  }
  return true;
}

double InvariantController::calcBandwidth(double sched_err, double start_err) {
  if (sched_err <= eps_max) {
    rsteps_inv_stat.addSample(rsteps_inv);	// Avoid imprecisions in accounting
    rsteps_inv = 0;
  } else
    rsteps_inv++;

  return calcBandwidthLimitedAsymm(sched_err, start_err);
}

/** Controller types ************************************************************/

Interval InvariantController::calcBandwidthRange(double sched_err, double start_err) {
  if (Interval(-eps_min, eps_max).contains(sched_err))
    eps_inv_stat.addSample(1.0);
  else
    eps_inv_stat.addSample(0.0);
  Interval c_range = getTaskExpInterval();
  if (c_range.isEmpty())
    return Interval(bw_max, bw_max);
  c_min = c_range.getMin();
  c_max = c_range.getMax();
  range_width_stat.addSample((c_max - c_min) / getTask()->getPeriod());
  range_alpha_stat.addSample(c_min / c_max);

  double period = getTask()->getPeriod();

  if (start_err > period - eps_min)
    return Interval(bw_max, bw_max);

  return calcBwRange(start_err, period, c_min, c_max, eps_min, eps_max);
}

/// Do not update any statistics or other internal state variable
Interval InvariantController::calcBandwidthRangeNoUpdate(double start_err) const {
  Interval c_range = getTaskExpInterval();
  if (c_range.isEmpty())
    return Interval(bw_max, bw_max);
  double c_min = c_range.getMin();
  double c_max = c_range.getMax();

  double period = getTask()->getPeriod();

  if (start_err > period - eps_min)
    return Interval(bw_max, bw_max);

  return calcBwRange(start_err, period, c_min, c_max, eps_min, eps_max);
}

double InvariantController::calcBandwidthLimitedAsymm(double sched_err, double start_err) {
  Interval br = calcBandwidthRange(sched_err, start_err);
  double bw = (br.getMin() + br.getMax()) / 2.0;
  if ((bw > bw_max) || (bw <= 0.0001))
    bw = bw_max;

  br = calcBandwidthRangeNoUpdate(eps_max);
  bw_prev_iot = (br.getMin() + br.getMax()) / 2.0;
  if ((bw_prev_iot > bw_max) || (bw_prev_iot <= 0.0001))
    bw_prev_iot = bw_max;

  return bw;
}

void InvariantController::dumpStats() {
  printf("# Prob{eps in [-e,E]} = %g (%ld samples)\n", eps_inv_stat.getMean(), eps_inv_stat.getNumSamples());

  parent::dumpStats();

  char *rsteps_inv_fname;
  rsteps_inv_fname = strdup("ri_stats0,0.dat");
  rsteps_inv_fname[8] = '0' + num_task;
  rsteps_inv_fname[10] = '0' + num_rs;
  rsteps_inv_stat.dumpStat(rsteps_inv_fname, "ri", true, "Return steps into invariant");

  /* Dump range width stats */
  char *rw_fname;
  rw_fname = strdup("rw_stats0,0.dat");
  rw_fname[8] = '0' + num_task;
  rw_fname[10] = '0' + num_rs;
  range_width_stat.dumpStat(rw_fname, "rw", false, "Prediction Range Width");

  /* Dump range alpha stats */
  char *ra_fname;
  ra_fname = strdup("ra_stats0,0.dat");
  ra_fname[8] = '0' + num_task;
  ra_fname[10] = '0' + num_rs;
  range_alpha_stat.dumpStat(ra_fname, "ra", false, "Prediction Range Alpha");
}

Interval InvariantController::calcBwRange(
  double start_err, double period,
  double c_min, double c_max,
  double eps_min, double eps_max
) {
  double bb;
  double BB;
  BB = c_min / (period - eps_min - start_err);
  bb = c_max / (period + eps_max - start_err);
  if (BB <= bb)
    BB = bb + DELTA;
  Logger::debugLog("Invariant-Based Bandwidth Range: "
      "se=%g, T=%g, c=%g, C=%g, e=%g, E=%g, bb=%g, BB=%g\n",
      start_err, period, c_min, c_max, eps_min, eps_max, bb, BB);
  return Interval(bb, BB);
}

/** End Controller types ********************************************************/
