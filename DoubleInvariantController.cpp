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

#include "util.hpp"
#include "defaults.hpp"
#include "DoubleInvariantController.hpp"
#include "Interval.hpp"
#include "DoubleLimitedTask.hpp"
#include "ResourceManager.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <math.h>

using namespace std;

#define MAX_O_LEN 10	/* Max length value for outside chain length statistics */

DoubleInvariantController::DoubleInvariantController()
  : parent(), stat_o_len(0, MAX_O_LEN, 1.0) {
//   setTask(Task::getInstance("p"));
  coeff_pos_i = DEF_P;
  eps_max_i = eps_min_i = UNASSIGNED;
  q.push_back(getTask()->generateWorkingTime());
  sample_size = 10;

  /* Statistic */
  curr_o_len = 0;
  n_eps_i = n_eps_o = 0;

  best_effort = true;
}

bool DoubleInvariantController::checkParams1() {
  double period = getTask()->getPeriod();
  if (! parent::checkParams())
    return false;
  BCHECK(eps_min_i + eps_max_i * c_min_i / c_max_i > period * (1 - c_min_i / c_max_i), "e_i+h_i/H_i*E_i <= T(1-h_i/H_i)");
  BCHECK(eps_min_i + eps_max * c_min / c_max > period * (1 - c_min / c_max), "e_i+h/H*E <= T(1-h/H)");
  BCHECK(eps_min + eps_max_i * c_min / c_max > period * (1 - c_min / c_max), "e+h/H*E_i <= T(1-h/H)");
  return true;
}

double DoubleInvariantController::getTaskInfExecutionTimeIRatio(void) {
  DoubleLimitedTask * p_tsk = dynamic_cast<DoubleLimitedTask *>(getTask());
  if (p_tsk != 0)
    return p_tsk->getInfExecutionTimeIRatio();
  return getTask()->getMinExecutionTime() / getTask()->getMaxExecutionTime();
}

bool DoubleInvariantController::checkParams() {
  if (! parent::checkParams())
    return false;

  double period = getTask()->getPeriod();

  if (strict_checks) {
    double alpha_i = getTaskInfExecutionTimeIRatio();

    BCHECK(period >= c_max / getMaxBandwidth(), "H / b_max > T");
    BCHECK(eps_min_i + eps_max_i * alpha_i > period * (1 - alpha_i), "e_i+a_i*E_i <= T(1-a_i)");

    /* Existence of double scheduler */
    BCHECK(period * (1/c_min - 1/c_max_i) <= eps_max_i/c_max_i + eps_min/c_min, "T(1/h-1/Hi) < Ei/Hi + e/h");
    BCHECK(period * (1/c_min_i - 1/c_max) <= eps_max/c_max + eps_min_i/c_min_i, "T(1/hi-1/H) < E/H + ei/hi");

    /* Exponential attractor to the internal invariant
    double alpha = c_min_i / c_max_i;
    double delta = u_min_i * c_max_i / period;
    double aa = (1 - alpha) / (1 + alpha);
    BCHECK(gamma > aa, "g<=(1-a)/(1+a)");
    BCHECK(gamma > delta / (2 - delta), "g<=d/(2-d)");
    */
  }
  return true;
}

/* Calculate scheduler dependent parameters */
void DoubleInvariantController::calcParams1() {
  parent::calcParams();
  DoubleLimitedTask *p_tsk = dynamic_cast<DoubleLimitedTask *>(getTask());
  CHECK(getTask() != 0, "Not a double limited task with double limited controller");
  c_min_i = p_tsk->getMinExecutionTimeI();
  c_max_i = p_tsk->getMaxExecutionTimeI();

  double ai = c_min_i / c_max_i;
  double min_sum_i = getTask()->getPeriod() * (1 - ai) + DELTA;
  //printf("# min_sum_i=%g\n", min_sum_i);
  eps_min_i = min_sum_i * (1 - coeff_pos_i) / (1 - (1 - ai) * coeff_pos_i);
  eps_max_i = eps_min_i * coeff_pos_i / (1 - coeff_pos_i);
}

/* Calculate scheduler dependent parameters */
void DoubleInvariantController::calcParams() {
  parent::calcParams();

  double period = getTask()->getPeriod();

  DoubleLimitedTask *p_tsk = dynamic_cast<DoubleLimitedTask *>(getTask());
  double ai;
  if (p_tsk != 0) {
    c_min_i = p_tsk->getInfMinExecutionTimeI();
    c_max_i = p_tsk->getSupMaxExecutionTimeI();
    ai = p_tsk->getInfExecutionTimeIRatio();
  } else {
    c_min_i = c_min;
    c_max_i = c_max;
    ai = c_min / c_max;
  }

  if (strict_checks) {
    double a = c_min / c_max;
    double b1 = c_min / c_max_i;
    double b2 = c_min_i / c_max;
  
    Logger::debugLog("# a=%g, b1=%g, b2=%g, ai=%g\n", a, b1, b2, ai);

    ASSERT(eps_min_i + eps_max_i * c_min_i / c_max_i >= 1 - c_min_i / c_max_i,
	    "ei + ai Ei < 1 - ai");
    ASSERT(a < b1, "a >= b1");
    ASSERT(a < b2, "a >= b2");
    ASSERT(b1 < ai, "b1 >= ai");
    ASSERT(b2 < ai, "b2 >= ai");
  }

  if ((eps_min_i == UNASSIGNED) || (eps_max_i == UNASSIGNED)) {
    double period = getTask()->getPeriod();
    calcInvariant(period, c_min_i / c_max_i, coeff_pos, eps_min_i, eps_max_i);
  }

  if (strict_checks) {
    ASSERT(eps_min + eps_max * c_min / c_max >= period * (1 - c_min / c_max),
	    "e + a E < 1 - a");
    ASSERT(eps_min_i + eps_max * c_min_i / c_max >= period * (1 - c_min_i / c_max),
	    "ei + hi/H E < 1 - hi/H");
    ASSERT(eps_min + eps_max_i * c_min / c_max_i >= period * (1 - c_min / c_max_i),
	    "e + h/Hi Ei < 1 - h/Hi");
    ASSERT((c_min_i >= c_min) && (c_max_i <= c_max), "Internal c(k) range not internal to full c(k) range");
    ASSERT((eps_min_i < eps_min) && (eps_max_i < eps_max), "Internal e(k) invariant not internal to full invariant");
  }
}

void DoubleInvariantController::usage() {
  printf("(-s di)    -cpi    Percentage of e(k) internal range that remains positive\n");
  printf("(-s di)    -gua    Invariant should be guaranteed during overloads\n");
}

bool DoubleInvariantController::parseArg(int& argc, char **& argv) {
  if (parent::parseArg(argc, argv)) {
    return true;
  } else if (strcmp(*argv, "-ei") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &eps_min) == 1, "Expecting double as argument to -e option");
  } else if (strcmp(*argv, "-Ei") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &eps_max) == 1, "Expecting double as argument to -E option");
  } else if (strcmp(*argv, "-cpi") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &coeff_pos_i) == 1, "Expecting double as argument to -cpi option");
    CHECK((coeff_pos_i >= 0) && (coeff_pos_i <= 1), "Expecting double in [0,1] range to -cpi option");
  } else if (strcmp(*argv, "-gua") == 0) {
    best_effort = false;
  } else {
    return false;
  }
  return true;
}

Interval DoubleInvariantController::getTaskExpIntervalI() {
  DoubleLimitedTask *p_tsk = dynamic_cast<DoubleLimitedTask *>(getTask());
  if (false && p_tsk != 0) {
    printf("Returning TASK minExecutionTimeI()\n");
    c_min_i = p_tsk->getMinExecutionTimeI();
    c_max_i = p_tsk->getMaxExecutionTimeI();
    return Interval(c_min_i, c_max_i);
  }
  return getTaskExpInterval();
}

Interval DoubleInvariantController::calcBwRangeI(double start_err) {
  return calcBwRange(start_err, getTask()->getPeriod(), c_min_i, c_max_i, eps_min_i, eps_max_i).intersect(Interval(0, bw_max));
}

double DoubleInvariantController::calcBandwidth(double sched_err, double start_err) {
  double h_i, H_i;
  //calcInternalInterval(h_i, H_i);
  Interval iv = getTaskExpIntervalI();
  h_i = iv.getMin();
  H_i = iv.getMax();
  c_min_i = h_i;
  c_max_i = H_i;
  //calcParams();

  double eps = start_err;

  if ((eps < -eps_min-DELTA) || (eps > eps_max+DELTA)) {
    Logger::debugLog("e(k) out of external boundaries\n");
  }

  ASSERT(checkParams(), "checkParams(k) failed !");

  //if (/*(-eps_min_i <= eps) &&*/ (eps <= eps_max_i)) {
    //double ci = p_tsk->getMinExecutionTimeI();
    //double Ci = p_tsk->getMaxExecutionTimeI();
    Interval bw_r_i = calcBwRange(eps, getTask()->getPeriod(), h_i, H_i, eps_min_i, eps_max_i);
    if (! bw_r_i.isEmpty())
      this->bw_range_i = Interval(MIN(bw_max, bw_r_i.getMin()), MIN(bw_max, bw_r_i.getMax()));
    Interval bw_r_e = calcBandwidthRange(sched_err, start_err);
    // Interval bw_r_e = calcBwRange(eps, getTask()->getPeriod(), c_min, c_max, eps_min, eps_max);
    if (! bw_r_e.isEmpty())
      this->bw_range = Interval(MIN(bw_max, bw_r_e.getMin()), MIN(bw_max, bw_r_e.getMax()));
    double bw = 1.0;
    if (! bw_r_i.isEmpty())
      bw = bw_r_i.getAvg();
    if (strict_checks) {
      ASSERT(! bw_r_i.isEmpty(), "Internal Invariant impossible");
      ASSERT(! bw_r_e.isEmpty(), "External Invariant impossible");
      Interval bw_r = bw_r_e.intersect(bw_r_i);
      ASSERT(! bw_r.isEmpty(), "Double Invariant impossible");
      bw_r = bw_r.intersect(Interval(0.0, bw_max));
      ASSERT(! bw_r.isEmpty(), "External limit makes double invariant impossible");
    }
    bw = MIN(bw, bw_r_e.getMax());
    bw = MIN(bw, bw_max);

    if (bw <= 0.001)
      return bw_max;

    return  bw;
//  } else {
//    //return u_min;
//    /*
//     eps(k)+c(k)u(k)-T >= -ei, with c(k) in [h_i, H_i]
//     u(k) >= [T-ei-eps(k)]/c(k)
//     */
//    Interval r_e = calcUIntervalLimitedAsymm(eps, period, c_min, c_max, eps_min, eps_max);
//    /* Avoid to surpass the invariant, i.e. to go to e(k+1) < -e_i from e(k) > E_i */
//    Interval r_e_i = Interval(MAX((period - eps_min_i - eps)/c_min_i, (period - eps_min_i - eps)/c_max_i), INF);
//    Interval rr = r_e.intersect(r_e_i).intersect(Interval(u_min, INF));
//    if (rr.isEmpty())
//      return 1.0 / u_min;
//    else
//      //return rr.getMax();
//      return 2.0 / (rr.getMin() + rr.getMax());
//
////     double ai = c_min_i/c_max_i;
////     //Logger::debugLog("e(k)=%g\n", eps);
//
////     double e_neg_max = period/gamma*(1-ai)/(1+ai);
////     double e_pos_min = period*(1-ai)/(1-ai+gamma*(1+ai));
////     double e_pos_max = period/(1-gamma);
////     //Logger::debugLog("eps_hole=[%g,%g] U [%g,%g] eps_i=[%g,%g]\n", -period, -e_neg_max, e_pos_min, e_pos_max, eps_min_i, eps_max_i);
////     bool b1 = ((-period < eps) && (eps < -e_neg_max));
////     bool b2 = ((e_pos_min < eps) && (eps < e_pos_max));
////     //Logger::debugLog("%d %d\n", b1, b2);
////     ASSERT((b1 || b2), "e(k) outside internal interval, but outside exponential control range");
//
////     Interval r_exp = calcUIntervalReduced(c_min_i, c_max_i, gamma);
////     ASSERT(! r_exp.isEmpty(), "Scheduling impossible");
////     Interval r = r_exp.intersect(Interval(u_min, INF));
////     if (r.isEmpty())
////       return u_min;
////     else
////       return r.getMin()+DELTA;
////     return LimitedController::calcBandwidthReduced();
//  }
}

double DoubleInvariantController::meanDistOut() {
  return stat_o_len.getMean();
}

void DoubleInvariantController::setLastJobExecTime(double c) {
  double pred_val = getTaskExpValue();
  if (pred_val > 0) {
    double pred_err = (pred_val - c) / getTask()->getPeriod();
    Logger::debugLog("Adding pred error sample: %g\n", pred_err);
    pe_stat.addSample(pred_err);
  }
  Interval iv = getTaskExpInterval();
  double c_min = iv.getMin();
  double c_max = iv.getMax();
  if (c_min > 0.0 && c_max > c_min) {
  	if (c_min <= c && c <= c_max)
  	  pr_stat.addSample(1.0);
  	else
  	  pr_stat.addSample(0.0);
  }

  getTaskPredictor()->addSample(c);

  Interval iv_i = getTaskExpIntervalI();
  double c_min_i = iv.getMin();
  double c_max_i = iv.getMax();

  if (pred_file == 0) {
	char *pf_fname;
	pf_fname = strdup("pred0,0.dat");
	pf_fname[4] = '0' + num_task;
	pf_fname[6] = '0' + num_rs;
  	pred_file = fopen(pf_fname, "w");
  	ASSERT(pred_file != 0, "Could not open file !");
  	fprintf(pred_file, "# %9s %11s %11s %11s %11s %11s\n",
  			"m_k", "c_k", "h_k", "H_k", "hi_k", "Hi_k");
  }
  fprintf(pred_file, "%11g %11g %11g %11g %11g %11g\n",
	  pred_val, c, c_min, c_max, c_min_i, c_max_i);
}
