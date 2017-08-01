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

#include "TaskPredictor.hpp"
#include "DoubleRangePredictor.hpp"

#include <string.h>
#include <math.h>

TaskPredictor::TaskPredictor() {
  p_vpred = ValuePredictor::getInstance("mm");
  p_rpred = RangePredictor::getInstance("sb");
  p_curr_rpred = p_rpred;
  p_rpred_i = 0;
  sample_size = 8;
  sum = 0.0;
  sum_sqr = 0.0;
  perfect_pred = false;
  perfect_pred_alpha = UNASSIGNED;
  perfect_pred_sample = 0.0;
}

void TaskPredictor::clearHistory() {
  q.clear();
  sum = 0.0;
  sum_sqr = 0.0;
  p_vpred->clearHistory();
  p_rpred->clearHistory();
}

void TaskPredictor::usage() {
  printf("    TASK PREDICTOR OPTIONS\n");
  printf("           -vp     Set value predictor type: sp/mm/mumm/fir/ol/vt\n");
  printf("           -rp     Set range predictor type: sb/olr/rt\n");
  printf("          -irp     Set internal range predictor type: sb/olr/rt\n");
  printf("          -srp     Stack range predictor after value predictor\n");
  printf("           -pp     Enable perfect prediction\n");
  printf("           -pp-a   Enable perfect prediction specified range alpha\n");
  ValuePredictor::usage();
  RangePredictor::usage();
}

bool TaskPredictor::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-vp") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    if (p_vpred != 0)
      delete p_vpred;
    p_vpred = ValuePredictor::getInstance(*argv);
    CHECK(p_vpred != 0, "Unknown value predictor type !");
  } else if (strcmp(*argv, "-rp") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    if (p_rpred != 0)
      delete p_rpred;
    p_rpred = RangePredictor::getInstance(*argv);
    CHECK(p_rpred != 0, "Unknown range predictor type !");
    p_curr_rpred = p_rpred;
  } else if (strcmp(*argv, "-irp") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    if (p_rpred_i != 0)
      delete p_rpred_i;
    p_rpred_i = RangePredictor::getInstance(*argv);
    CHECK(p_rpred_i != 0, "Unknown range predictor type !");
    p_curr_rpred = p_rpred_i;
  } else if (strcmp(*argv, "-srp") == 0) {
    this->stack_rp = true;
  } else if (strcmp(*argv, "-pp") == 0) {
    this->perfect_pred = true;
  } else if (strcmp(*argv, "-pp-a") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &perfect_pred_alpha) == 1, "Expecting double as argument to -pp-a option");
    CHECK((perfect_pred_alpha > 0.0) && (perfect_pred_alpha < 1.0), "Expecting double in the range ]0,1[ as argument to -pp-a option");
    perfect_pred = true;
  } else if (p_vpred != 0 && p_vpred->parseArg(argc, argv))
    return true;
  else if (p_curr_rpred != 0 && p_curr_rpred->parseArg(argc, argv))
    return true;
  else
    return false;
  return true;
}

void TaskPredictor::calcParams() {
  ASSERT(p_vpred != 0, "No value predictor instantiated for task predictor");
  p_vpred->calcParams();
  ASSERT(p_rpred != 0, "No range predictor instantiated for task predictor");
  p_rpred->calcParams();
}

/** Add a c_k sample */
void TaskPredictor::addSample(double sample) {
  if (! stack_rp) {
    p_vpred->addSample(sample);
    p_rpred->addSample(sample);
    if (p_rpred_i != 0)
      p_rpred_i->addSample(sample);
  } else {
    double old_pred = p_vpred->getExpValue();
    p_vpred->addSample(sample);
    if (old_pred != 0 && old_pred != -1) {
      p_rpred->addSample(sample - old_pred);
      if (p_rpred_i != 0)
        p_rpred_i->addSample(sample - old_pred);
    }
  }

  q.push_back(sample);
  sum += sample;
  sum_sqr += sample * sample;
  if ((int) q.size() > sample_size) {
    double c_old = *(q.begin());
    // Logger::debugLog("Removing sample %g\n", c_old);
    q.pop_front();
    sum -= c_old;
    sum_sqr -= c_old * c_old;
  }
}

/** Return expected next value */
double TaskPredictor::getExpValue() const {
  if (! perfect_pred) {
    return p_vpred->getExpValue();
  } else {
    Logger::debugLog("Returning perfectly predicted sample: %g\n", perfect_pred_sample);
    return perfect_pred_sample;
  }
}

/** Return range in which next sample would reside with high probability */
Interval TaskPredictor::getExpInterval() const {
  if (! stack_rp) {
    Interval rpred_iv = p_rpred->getExpInterval();
    if (! perfect_pred)
      return rpred_iv;
    else {
      Interval iv;
      if (perfect_pred_alpha == UNASSIGNED) {
        // Center predicted range around (perfectly predicted) next sample
        iv = rpred_iv - rpred_iv.getAvg() + perfect_pred_sample;
      } else {
        // ( c * sqrt(a) ) / ( c / sqrt(a) ) == a
        iv = Interval(perfect_pred_sample * sqrt(perfect_pred_alpha), perfect_pred_sample / sqrt(perfect_pred_alpha));
      }
      Logger::debugLog("Returning perfectly predicted range: [%g,%g] (was: [%g,%g])\n",
          iv.getMin(), iv.getMax(), rpred_iv.getMin(), rpred_iv.getMax());
      ASSERT(iv.contains(perfect_pred_sample), "Perfect prediction range does not contain next sample");
      return iv;
    }
  } else {
    double exp_value = getExpValue();
    Logger::debugLog("Evaluating expected pred error interval\n");
    Interval rpred_iv = p_rpred->getExpInterval();
    Logger::debugLog("range pred iv: [%g,%g] (to be shifted by exp_value=%g)\n", rpred_iv.getMin(), rpred_iv.getMax(), exp_value);
    if (exp_value == -1 || rpred_iv.isEmpty())
      return Interval();
    if (! perfect_pred)
      return rpred_iv + exp_value;
    else {
      Interval iv;
      if (perfect_pred_alpha == UNASSIGNED)
        // Center predicted range around (perfectly predicted) next sample
        iv = rpred_iv - rpred_iv.getAvg() + perfect_pred_sample;
      else {
        // ( c * sqrt(a) ) / ( c / sqrt(a) ) == a
        iv = Interval(perfect_pred_sample * sqrt(perfect_pred_alpha), perfect_pred_sample / sqrt(perfect_pred_alpha));
      }
      Logger::debugLog("Returning perfectly predicted range: [%g,%g] (was: [%g,%g])\n",
          iv.getMin(), iv.getMax(), rpred_iv.getMin(), rpred_iv.getMax());
      ASSERT(iv.contains(perfect_pred_sample), "Perfect prediction range does not contain next sample");
      return iv;
    }
  }
}

/** Return range in which next sample would reside with high probability */
Interval TaskPredictor::getExpIntervalI() const {
  Interval iv;
  if (p_rpred_i != 0) {
    iv = p_rpred_i->getExpInterval();
  } else {
    DoubleRangePredictor *p_drpred = dynamic_cast<DoubleRangePredictor*>(p_rpred);
    if (p_drpred != 0)
      iv = p_drpred->getExpIntervalI();
    else
      iv = p_rpred->getExpInterval();
  }
  if (iv.isEmpty())
    return Interval();
  if (! stack_rp) {
    return iv;
  } else {
    double exp_value = getExpValue();
    Logger::debugLog("internal range pred iv: [%g,%g]\n", iv.getMin(), iv.getMax());
    return iv + exp_value;
  }
}

/** Return standard deviation (unbiased estimation) **/
double TaskPredictor::getDevValue() const {
  if (q.size() < 2)
    return -1;
  double var = (sum_sqr - sum*sum/q.size()) / (q.size() - 1);
  return sqrt(var);
}

///** Return positive deviation */
//double TaskPredictor::getDevPosValue() {
//  return p_rpred->getDevPosValue();
//}

///** Return negative deviation */
//double TaskPredictor::getDevNegValue() {
//  return p_rpred->getDevNegValue();
//}
