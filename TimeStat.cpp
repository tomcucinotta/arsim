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

#include "TimeStat.hpp"
#include "Events.hpp"
#include "util.hpp"

#include <math.h>

TimeStat::TimeStat(double x_min, double x_max, long x_pmf_size, double now) {
  this->x_min = x_min;
  this->x_max = x_max;
  this->dx = (x_max - x_min) / x_pmf_size;
  this->x_pmf_size = x_pmf_size;
  x_pmf = new double[x_pmf_size];
  ASSERT(x_pmf != 0, "No memory");
  for (long n = 0; n < x_pmf_size; n++)
    x_pmf[n] = 0.0;
  num_samples = 0;
  x_sum = 0;
  x_sqr_sum = 0;
  x_abs_sum = 0;
  x_sum_pos = 0;
  t_sum_pos = 0;
  x_sum_neg_zero = 0;
  t_sum_neg_zero = 0;
  prev_t = now;
  orig_t = now;
  prev_x = 0.0;
}

TimeStat::TimeStat(double x_min, double x_max, double dx, double now)
{
  this->x_min = x_min;
  this->x_max = x_max;
  this->dx = dx;
  this->x_pmf_size = long(ceil((x_max - x_min) / dx));
  x_pmf = new double[x_pmf_size];
  ASSERT(x_pmf != 0, "No memory");
  for (long n = 0; n < x_pmf_size; n++)
    x_pmf[n] = 0.0;
  num_samples = 0;
  x_sum = 0;
  x_sqr_sum = 0;
  x_abs_sum = 0;
  x_sum_pos = 0;
  t_sum_pos = 0;
  x_sum_neg_zero = 0;
  t_sum_neg_zero = 0;
  prev_t = now;
  orig_t = now;
  prev_x = 0.0;
}

TimeStat::~TimeStat() {
  delete[] x_pmf;
}

double TimeStat::calcPMFMean() const {
  double sum = 0.0;
  for (long n = 0; n < x_pmf_size; n++)
    sum += x_pmf[n] / (prev_t - orig_t) * (x_min + dx * double(n));
  return sum;
}

double TimeStat::getMean() const {
  if (prev_t - orig_t == 0.0)
    return prev_x;
  return x_sum / (prev_t - orig_t);
}

double TimeStat::getMeanPos() const {
  if (t_sum_pos == 0.0 && prev_x >= 0.0)
    return prev_x;
  return x_sum_pos / t_sum_pos;
}

double TimeStat::getMeanNegZero() const {
  if (t_sum_pos == 0.0 && prev_x <= 0.0)
    return prev_x;
  return x_sum_neg_zero / t_sum_neg_zero;
}

double TimeStat::getDev() const {
  if (prev_t - orig_t == 0.0)
    return 0;
  double exp_x2 = x_sqr_sum / (prev_t - orig_t);
  double mean = getMean();
  return sqrt(exp_x2 - mean*mean);
}

double TimeStat::getMeanAbs() const {
  if (prev_t - orig_t == 0.0)
    return fabs(prev_x);
  return x_abs_sum / (prev_t - orig_t);
}

double TimeStat::getPDFValue(double x) const {
  long n_sample = long((x - x_min) / dx);
  ASSERT1((n_sample >= 0) && (n_sample < x_pmf_size), "TimeStat::getDistrAt(): x out of range: %g", x);
  return x_pmf[n_sample] / (dx * (prev_t - orig_t));
}

double TimeStat::getPMFValue(long n_sample) const {
  ASSERT1((n_sample >= 0) && (n_sample < x_pmf_size), "getPMFValue(): n_sample out of range: %ld", n_sample);
  return x_pmf[n_sample] / (prev_t - orig_t);
}

double TimeStat::getXValue(long n_sample) const {
  ASSERT1((n_sample >= 0) && (n_sample < x_pmf_size), "getPMFValue(): n_sample out of range: %ld", n_sample);
  return (x_min + (dx * double(n_sample)));
}

double TimeStat::sumPMFValues() const {
  double sum = 0.0;
  for (long n = 0; n < x_pmf_size; n++)
    sum += x_pmf[n];
  return sum / (prev_t - orig_t);
}

void TimeStat::clear(double now) {
  parent::clear();
  num_samples = 0;
  for (long n = 0; n < x_pmf_size; n++)
    x_pmf[n] = 0;
  x_sum = 0;
  x_sqr_sum = 0;
  x_abs_sum = 0;
  x_sum_pos = 0;
  t_sum_pos = 0;
  x_sum_neg_zero = 0;
  t_sum_neg_zero = 0;
  prev_t = now;
  orig_t = now;
  prev_x = 0;
}
