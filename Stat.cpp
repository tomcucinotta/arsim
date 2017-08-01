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

#include "Stat.hpp"
#include "util.hpp"

#include <math.h>

Stat::Stat(double x_min, double x_max, long x_pmf_size) {
  this->x_min = x_min;
  this->x_max = x_max;
  this->dx = (x_max - x_min) / double(x_pmf_size);
  this->x_pmf_size = x_pmf_size;
  x_pmf = new long[x_pmf_size];
  ASSERT(x_pmf != 0, "No memory");
  for (long n = 0; n < x_pmf_size; n++)
    x_pmf[n] = 0;
  num_samples = 0;
  x_sum = 0;
  x_sqr_sum = 0;
  x_abs_sum = 0;
  x_pos_sum = 0;
  num_pos = 0;
  x_neg_zero_sum = 0;
  num_neg_zero = 0;
  Logger::debugLog("pmf size: %ld\n", x_pmf_size);
}

Stat::Stat(double x_min, double x_max, double dx) {
  this->x_min = x_min;
  this->x_max = x_max;
  this->dx = dx;
  this->x_pmf_size = long(ceil((x_max - x_min) / dx));
  x_pmf = new long[x_pmf_size];
  ASSERT(x_pmf != 0, "No memory");
  for (long n = 0; n < x_pmf_size; n++)
    x_pmf[n] = 0;
  num_samples = 0;
  x_sum = 0;
  x_sqr_sum = 0;
  x_abs_sum = 0;
  x_pos_sum = 0;
  num_pos = 0;
  x_neg_zero_sum = 0;
  num_neg_zero = 0;
  Logger::debugLog("pmf size: %ld\n", x_pmf_size);
}

void Stat::clear() {
  parent::clear();
  num_samples = 0;
  for (long n = 0; n < x_pmf_size; n++)
    x_pmf[n] = 0;
  x_sum = 0;
  x_sqr_sum = 0;
  x_abs_sum = 0;
  x_pos_sum = 0;
  num_pos = 0;
  x_neg_zero_sum = 0;
  num_neg_zero = 0;
}

Stat::~Stat() {
  delete[] x_pmf;
}

double Stat::calcPMFMean() const {
  double sum = 0.0;
  for (long n = 0; n < x_pmf_size; n++)
    sum += double(x_pmf[n])/double(num_samples) * (x_min + dx * double(n));
  return sum;
}

double Stat::getMean() const {
  return x_sum / double(num_samples);
}

double Stat::getMeanAbs() const {
  return x_abs_sum / double(num_samples);
}

double Stat::getMeanPos() const {
  return x_pos_sum / double(num_pos);
}

double Stat::getMeanNegZero() const {
  return x_neg_zero_sum / double(num_neg_zero);
}

double Stat::getDev() const {
  double exp_x2 = x_sqr_sum / double(num_samples);
  double mean = getMean();
  return sqrt(exp_x2 - mean*mean);
}

double Stat::getPDFValue(double x) const {
  long n_sample = long((x - x_min) / dx);
  if (! ((n_sample >= 0) && (n_sample < x_pmf_size)) ) {
    fprintf(stderr, "x_min=%g, x_max=%g, dx=%g, size=%ld\n",
	   x_min, x_max, dx, x_pmf_size);
    ASSERT1((n_sample >= 0) && (n_sample < x_pmf_size), "Stat::getDistrAt(): x out of range: %g", x);
  }
  return double(x_pmf[n_sample])/(dx*double(num_samples));
}

double Stat::getPMFValue(long n_sample) const {
  ASSERT1((n_sample >= 0) && (n_sample < x_pmf_size), "getPMFValue(): n_sample out of range: %ld", n_sample);
  return double(x_pmf[n_sample])/double(num_samples);
}

double Stat::getXValue(long n_sample) const {
  ASSERT1((n_sample >= 0) && (n_sample < x_pmf_size), "getPMFValue(): n_sample out of range: %ld", n_sample);
  return (x_min + (dx * double(n_sample)));
}

double Stat::sumPMFValues() const {
  long sum = 0;
  for (long n = 0; n < x_pmf_size; n++)
    sum += x_pmf[n];
  return double(sum)/double(num_samples);
}
