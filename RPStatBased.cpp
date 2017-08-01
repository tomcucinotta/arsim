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

#include "RPStatBased.hpp"

#include "Stat.hpp"

#include <stdio.h>
#include <string.h>
#include <algorithm>

RPStatBased::RPStatBased() {
  sample_size = 12;
  percentile = 0.83;
}

void RPStatBased::clearHistory() {
  q.clear();
}

bool RPStatBased::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-sb-ss") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    sample_size = atoi(*argv);
    CHECK(sample_size > 0, "Expecting a positive integer as argument to -sb-ss option");
  } else if (strcmp(*argv, "-sb-p") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%lg", &percentile) == 1) && (percentile > 0.0 && percentile <= 1.0), "Expecting positive real as argument to -sb-p option");
  } else
    return false;
  return true;
}

void RPStatBased::usage() {
  printf("(-rp sb)   -sb-ss  Sample size for range predictor\n");
  printf("(-rp sb)   -sb-p   Set percentile at which to cut range\n");
}

/* Add a c_k sample */
void RPStatBased::addSample(double c_k) {
  q.push_back(c_k);
  if ((int) q.size() > sample_size) {
    double c_old = *(q.begin());
    Logger::debugLog("# RPStatBased::addSample() - Removing sample %g\n", c_old);
    q.pop_front();
  }
  Logger::debugLog("# RPStatBased::addSample() - After addition: sample_size=%d, q.size=%d\n", sample_size, q.size());
}

/* Return range in which next sample would reside with high probability */
Interval RPStatBased::getExpInterval() {
  Logger::debugLog("Queue dump: ");
  for (deque<double>::const_iterator it = q.begin(); it != q.end(); ++it)
    Logger::debugLog("%g, ", *it);
  Logger::debugLog("\n");

  if (q.size() == 0)
    return Interval();

//  double a = *min_element(q.begin(), q.end());
//  double b = *max_element(q.begin(), q.end());
//  ASSERT(a != MAXDOUBLE && b != -MAXDOUBLE, "Bah !");
//  if (q.size() <= 2) {
//    Logger::debugLog("Computed percentiles (min/max): [%g, %g]\n", a, b);
//    return Interval(a, b);
//  }

//  double min = Stat::getMinPercentile(q, percentile, a, b);
//  double max = Stat::getMaxPercentile(q, percentile, a, b);

  std::vector<double> v(q.begin(), q.end());
  std::sort(v.begin(), v.end());

  Logger::debugLog("Ordered queue dump: ");
  for (vector<double>::const_iterator it = v.begin(); it != v.end(); ++it)
    Logger::debugLog("%g, ", *it);
  Logger::debugLog("\n");

  // If I didn't make mistakes, this way the interval is always chosen symmetrically w.r.t. extremes of v[]
  int discarded = round((1.0 - percentile) * v.size());
  discarded = std::min<int>((v.size()-1)/2, discarded);
  int min_idx = discarded;
  int max_idx = v.size() - 1 - discarded;
  Logger::debugLog("min_idx=%d, max_idx=%d\n", min_idx, max_idx);
  ASSERT(min_idx >= 0 && min_idx < (int) v.size(), "min_idx out of range");
  ASSERT(max_idx >= 0 && max_idx < (int) v.size(), "max_idx out of range");
  ASSERT(min_idx <= max_idx, "min_idx > max_idx");

  double min = v[min_idx];
  double max = v[max_idx];

  Logger::debugLog("Computed percentiles: [%g, %g]\n", min, max);
  return Interval(min, max);
}

void RPStatBased::setSampleSize(int new_size) {
  sample_size = new_size;
  clearHistory();
}

void RPStatBased::setPercentile(double new_perc) {
  percentile = new_perc;
  clearHistory();
}

///* Approximate standard deviation as width of prediction interval */
//double RPStatBased::getDevValue() {
//  return curr_iv.getMax() - curr_iv.getMin();
//}
//
///* Approximate positive deviation as half width of prediction interval */
//double RPStatBased::getDevPosValue() {
//  return (curr_iv.getMax() - curr_iv.getMin()) / 2.0;
//}
//
///* Approximate negative deviation as half width of prediction interval, negated */
//double RPStatBased::getDevNegValue() {
//  return - (curr_iv.getMax() - curr_iv.getMin()) / 2.0;
//}
