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

#include "Controller.hpp"
#include "LimitedController.hpp"
#ifdef WITH_DOUBLE_LIMITED
#  include "LimitedControllerK.hpp"
#  include "DoubleInvariantController.hpp"
#endif
#include "InvariantController.hpp"
#include "FixedController.hpp"
#include "SDBController.hpp"
#include "PDNVController.hpp"
#include "PIController.hpp"
#include "MSSEController.hpp"
#include "OCController.hpp"
#include "util.hpp"
#include "defaults.hpp"
#include "ResourceManager.hpp"

#include "TaskScheduler.hpp"

/* Implementation includes */

#include <string.h>
#include <math.h>

Controller::Controller() :
  pe_stat(-1.0, 1.0, 0.01),
  pr_stat(0.0, 2.0, 1.0)
{
  curr_k = 0;
  c_prev = 0;
  bw_prev_iot = 1.0;
  bw_max = DEF_BW_MAX;
  bw_min = DEF_BW_MIN;

  p_tpred = new TaskPredictor();
  ASSERT(p_tpred != 0, "No more memory");

  dyn_bw_active = false;
  dyn_bw_rel_dl = 0.0;

  pred_file = 0;

  p_task = new Task();
  ASSERT(p_task != 0, "No memory for new task");

  num_task = 0;
  num_rs = 0;
}

Controller::~Controller() {
  if (p_task != 0)
    delete p_task;
}

Controller * Controller::getInstance(const char *s) {
  Controller *p_sched = 0;
  LimitedController *p_lsched = 0;
  if (strcmp(s, "re") == 0) {
    p_lsched = new LimitedController();
    p_lsched->setMethod(&LimitedController::calcBandwidthReduced);
    p_sched = p_lsched;
  } else if (strcmp(s, "la") == 0) {
    p_lsched = new LimitedController();
    p_lsched->setMethod(&LimitedController::calcBandwidthLimitedAsymm);
    p_sched = p_lsched;
#ifdef WITH_DOUBLE_LIMITED
   } else if (strcmp(s, "lak") == 0) {
     p_sched = new LimitedControllerK();
   } else if (strcmp(s, "di") == 0) {
     p_sched = new DoubleInvariantController();
#endif
  } else if (strcmp(s, "ib") == 0) {
    p_sched = new InvariantController();
  } else if (strcmp(s, "ey") == 0) {
    p_lsched = new LimitedController();
    p_lsched->setMethod(&LimitedController::calcBandwidthLimitedEY);
    p_sched = p_lsched;
  } else if (strcmp(s, "fs") == 0) {
    p_sched = new FixedController();
  } else if (strcmp(s, "sdb") == 0) {
    p_sched = new SDBController();
  } else if (strcmp(s, "pdnv") == 0) {
    p_sched = new PDNVController();
  } else if (strcmp(s, "pi") == 0) {
    p_sched = new PIController();
  } else if (strcmp(s, "msse") == 0) {
    p_sched = new MSSEController();
  } else if (strcmp(s, "oc") == 0) {
    p_sched = new OCController();
  }
  return p_sched;
}

bool Controller::checkParams() {
  Logger::debugLog("Checking controller params\n");
  ASSERT(p_task != 0, "No task instantiated for controller");
  return p_task->checkParams();
}

void Controller::calcParams() {
  Logger::debugLog("Computing controller params\n");
  ASSERT(p_task != 0, "No task instantiated for controller");
  p_task->calcParams();
  ASSERT(this->p_tpred != 0, "No task predictor instantiated for controller");
  p_tpred->calcParams();
}

void Controller::usage() {
  TaskPredictor::usage();
  printf("           -B      Maximum bandwidth (default: %g)\n", DEF_BW_MAX);
  printf("           -b      Minimum guaranteed bandwidth (default: %g)\n",
	 DEF_BW_MIN);
  LimitedController::usage();
#ifdef WITH_DOUBLE_LIMITED
  DoubleInvariantController::usage();
  LimitedControllerK::usage();
#endif
  InvariantController::usage();
  SDBController::usage();
  MSSEController::usage();
  OCController::usage();
  PIController::usage();
}

bool Controller::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-B") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &bw_max) == 1, "Expecting double as argument to -B option");
    CHECK((bw_max <= 1.0) && (bw_max > 0.0), "Expecting double in the range ]0,1] as argument to -B option");
  } else if (strcmp(*argv, "-b") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &bw_min) == 1, "Expecting double as argument to -b option");
    CHECK((bw_min <= 1.0) && (bw_min >= 0.0), "Expecting double in the range [0,1] as argument to -b option");
  } else if (strcmp(*argv, "-dyn-bw") == 0) {
    dyn_bw_active = true;
  } else if ((p_task != 0) && p_task->parseArg(argc, argv)) {
    return true;
  } else if ((p_tpred != 0) && p_tpred->parseArg(argc, argv)) {
    return true;
  } else
    return false;
  return true;
}

double Controller::getTaskExpValue() const {
  double ret = p_tpred->getExpValue();
  if (ret == -1) {
    ret = (p_task->getMinExecutionTime() + p_task->getMaxExecutionTime()) / 2.0;
    Logger::debugLog("Predictor unavailable yet; min and max from task averaged to: %lg\n", ret);
  }
  return ret;
}

double Controller::getTaskDev() {
  double ret = p_tpred->getDevValue();
  if (ret == -1)
    ret = (p_task->getMaxExecutionTime() - p_task->getMinExecutionTime()) / sqrt(12.0); // Dev of a constant pdf
  return ret;
}

//double Controller::getTaskDevPos() {
//  double ret = p_tpred->getDevPosValue();
//  if (ret == -1)
//    ret = (p_task->getMaxExecutionTime() - p_task->getMinExecutionTime()) / 2.0; // Dev of a constant pdf
//  return ret;
//}
//
//double Controller::getTaskDevNeg() {
//  double ret = p_tpred->getDevNegValue();
//  if (ret == -1)
//    ret = (p_task->getMinExecutionTime() - p_task->getMaxExecutionTime()) / 2.0; // Dev of a constant pdf
//  return ret;
//}

Interval Controller::getTaskExpInterval() const {
  Interval iv = p_tpred->getExpInterval();
  double c_min = iv.getMin();
  double c_max = iv.getMax();

  Logger::debugLog("Saturating pred range [%g,%g] accounting for c_min=%g, c_max=%g\n",
      c_min, c_max, getTask()->getMinExecutionTime(), getTask()->getMaxExecutionTime());
  if (getTask()->getMinExecutionTime() != UNASSIGNED)
    if (iv.isEmpty() || c_min < getTask()->getMinExecutionTime())
      c_min = getTask()->getMinExecutionTime();
//  if (c_min < getTask()->getMinExecutionTime())
//    c_min = getTask()->getMinExecutionTime();
  if (getTask()->getMaxExecutionTime() != UNASSIGNED)
    if (iv.isEmpty() || c_max > getTask()->getMaxExecutionTime())
      c_max = getTask()->getMaxExecutionTime();
  if (c_min > c_max)
    std::swap(c_min, c_max);
  Logger::debugLog("Saturated predicted range: [%g,%g]\n", c_min, c_max);

  return Interval(c_min, c_max);
}

void Controller::setLastJobExecTime(double c) {
  double pred_val = getTaskExpValue();
  if (pred_val > 0) {
    double pred_err = (pred_val - c) / p_task->getPeriod();
    Logger::debugLog("Adding pred error sample: %g (relative), %g (absolute)\n",
        pred_err, pred_err * p_task->getPeriod());
    pe_stat.addSample(pred_err);
  }
  Interval I = getTaskExpInterval();
  double c_min = I.getMin();
  double c_max = I.getMax();
  if (/*c_min > 0.0 &&*/ c_max >= c_min) {
    if (/*c_min <= c &&*/ c <= c_max)
      pr_stat.addSample(1.0);
    else
      pr_stat.addSample(0.0);
  }
  if (curr_k > 15 && (I.isEmpty() || fabs(c_min) > 1000000 || fabs(c_max) > 1000000)) {
    fprintf(stderr, "# Empty prediction range: [%g,%g]\n", I.getMin(), I.getMax());
    I = getTaskExpInterval();
    fprintf(stderr, "# Empty prediction range: [%g,%g]\n", I.getMin(), I.getMax());
  }
  p_tpred->addSample(c);

  if (pred_file == 0) {
	char *pf_fname;
	pf_fname = strdup("pred0,0.dat");
	pf_fname[4] = '0' + num_task;
	pf_fname[6] = '0' + num_rs;
  	pred_file = fopen(pf_fname, "w");
  	ASSERT(pred_file != 0, "Could not open file !");
  	fprintf(pred_file, "# %9s %11s %11s %11s\n",
  			"m_k", "c_k", "h_k", "H_k");
  }
  fprintf(pred_file, "%11g %11g %11g %11g\n",
	  pred_val, c, c_min, c_max);
  ++curr_k;
}

void Controller::dumpStats() {
  /** File where to dump pred err stats to	*/
  char *pe_fname;
  pe_fname = strdup("pe_stats0,0.dat");
  pe_fname[8] = '0' + num_task;
  pe_fname[10] = '0' + num_rs;
  pe_stat.dumpStat(pe_fname, "pe", false, "Prediction Error");

  /** Dump stats of correct range prediction */
  char *pr_fname;
  pr_fname = strdup("pr_stats0,0.dat");
  pr_fname[8] = '0' + num_task;
  pr_fname[10] = '0' + num_rs;
  pr_stat.dumpStat(pr_fname, "pr", true, "Correct Prediction Range");
}

void Controller::setTask(Task* p_t) {
  ASSERT(p_t != 0, "Null pointer passed in as new task");
  if (p_task != 0)
    delete p_task;
  p_task = p_t;
}
