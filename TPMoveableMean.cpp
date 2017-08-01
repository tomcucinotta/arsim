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

#include <cstring>

#include "TPMoveableMean.hpp"

#include "util.hpp"
#include "Interval.hpp"

#include <math.h>

TPMoveableMean::TPMoveableMean() {
  sample_size = 8;
  sum = 0.0;
  discard_old_samples = false;
}

void TPMoveableMean::clearHistory() {
  q.clear();
  sum = 0.0;
}

bool TPMoveableMean::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-mm-ss") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    sample_size = atoi(*argv);
    CHECK(sample_size > 0, "Expecting a positive integer as argument to -mm-ss option");
  } else if (strcmp(*argv, "-dod") == 0) {
    discard_old_samples = true;
  } else
    return false;
  return true;
}

void TPMoveableMean::usage() {
  printf("(-tp mm)   -mm-ss  Sample size for moving average\n");
  printf("(-tp mm)   -dod    Enable discarding of old samples\n");
}

/* Add a c_k sample */
void TPMoveableMean::addSample(double c_k) {
  /* If previous mean is too different, then discard an old sample */
  double c_mean = getExpValue();
  Logger::debugLog("# TPMoveableMean::addSample() - Adding sample %lg\n", c_k);
  if (discard_old_samples && (c_mean != -1) && (fabs(c_k - c_mean) > 0.6) && (q.size() >= 4)) {
    Logger::debugLog("# TPMoveableMean::addSample() - DISCARDING OLD SAMPLE\n");
    double c_old = *(q.begin());
    q.pop_front();
    sum -= c_old;
  }
  q.push_back(c_k);
  sum += c_k;
  if ((int) q.size() > sample_size) {
    double c_old = *(q.begin());
    // Logger::debugLog("Removing sample %g\n", c_old);
    q.pop_front();
    sum -= c_old;
  }
  Logger::debugLog("# TPMoveableMean::addSample() - After addition: sample_size=%d, q.size=%d\n", sample_size, q.size());
}

//  /* Return range in which next sample would reside with high probability */
//Interval TPMoveableMean::getExpInterval() {
//  double c_mean = getExpValue();
//  if (c_mean == -1)
//    return Interval();
//  double dev_neg = getDevNegValue();
//  double dev_pos = getDevPosValue();
//  double c_min;
//  if (dev_neg == -1)
//    c_min = c_mean * 0.75;
//  else
//    c_min = c_mean + 1.0 * dev_neg;
//  double c_max;
//  if (dev_pos == -1)
//    c_max = c_mean * 1.25;
//  else
//    c_max = c_mean + 1.0 * dev_pos;
//
//  Interval iv(c_min, c_max);
//
//  if (q.size() > 0) {
//    int correct_preds = 0;
//    for (unsigned int i = 0; i < q.size(); i++)
//      if (iv.contains(q[i]))
//	  correct_preds++;
//    Logger::debugLog("Returning prediction interv [%g,%g] (alpha=%g) w/rate: %02.2f%%\n",
//		     c_min, c_max, c_min / c_max, correct_preds * 100.0 / q.size());
//    //fprintf(stderr, "Returning prediction interv [%g,%g] (alpha=%g) w/rate: %02.2f%%\n",
//	//	     c_min, c_max, c_min / c_max, correct_preds * 100.0 / q.size());
//  }
//  return iv;
//}

  /* Return expected next value */
double TPMoveableMean::getExpValue() const {
  if (q.size() == 0)
    return -1;
  return sum / q.size();
}

//  /* Return std deviation */
//double TPMoveableMean::getDevValue() {
//  if (q.size() == 0)
//    return -1;
//  /* TODO: Exclude min and max from calculus */
//  deque<double>::iterator it;
//  double sum_sqr = 0.0;
//  double mean = getExpValue();
//  for (it = q.begin(); it != q.end(); it++)
//    sum_sqr += (*it - mean) * (*it - mean);
//  return sqrt(sum_sqr / q.size());
//}
//
//double TPMoveableMean::getDevPosValue() {
//  if (q.size() < 3)
//    return -1;
//  int cnt_pos = 0;
//  deque<double>::iterator it;
//  double sum = 0.0;
//  double mean = getExpValue();
//  for (it = q.begin(); it != q.end(); ++it) {
//    double x = (*it - mean);
//    if (x >= 0.0) {
//      sum += x;
//      cnt_pos++;
//    }
//  }
//  if (cnt_pos >= 1)
//    old_dev_pos = sum / cnt_pos;
//  return old_dev_pos;
//}
//
//double TPMoveableMean::getDevNegValue() {
//  if (q.size() < 3)
//    return -1;
//  /* TODO: Exclude min and max from calculus */
//  int cnt_neg = 0;
//  deque<double>::iterator it;
//  double sum = 0.0;
//  double mean = getExpValue();
//  for (it = q.begin(); it != q.end(); ++it) {
//    double x = (*it - mean);
//    if (x <= 0.0) {
//      sum += x;
//      cnt_neg++;
//    }
//  }
//  if (cnt_neg >= 1)
//    old_dev_neg = sum / cnt_neg;
//  return old_dev_neg;
//}

void TPMoveableMean::setSampleSize(int new_size) {
  ASSERT(new_size > 0, "Wrong new sample size");
  sample_size = new_size;
}
