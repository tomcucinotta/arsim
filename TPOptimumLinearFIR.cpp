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

#include "TPOptimumLinearFIR.hpp"
#include "util.hpp"

#include <math.h>

#define DEF_OL_M 12
#define DEF_OL_H 1200
#define DEF_OL_L 1.0

TPOptimumLinearFIR::TPOptimumLinearFIR()
  : mm() {
  M = DEF_OL_M;
  H = DEF_OL_H;
  lambda = DEF_OL_L;
  num_samples = 0;
  recursive = false;
  mm.setSampleSize(M+H);
  last_exp_value = 0;
}

bool TPOptimumLinearFIR::parseArg(int& argc, char **& argv) {
  Logger::debugLog("Parsing option: %s\n", *argv);
  if (strcmp(*argv, "-ol-l") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%lg", &lambda) == 1) && (lambda > 0.0) && (lambda <= 1.0), "Expecting real in 0..1 range as argument to -ol-l option");
  } else if (strcmp(*argv, "-ol-m") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%d", &M) == 1) && (M > 0), "Expecting positive integer as argument to -ol-m option");
    mm.setSampleSize(M+H);
  } else if (strcmp(*argv, "-ol-h") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%d", &H) == 1) && (H > 0), "Expecting positive integer as argument to -ol-h option");
    mm.setSampleSize(M+H);
  } else if (strcmp(*argv, "-ol-r") == 0) {
    recursive = true;
  } else
    return parent::parseArg(argc, argv);
  return true;
}

void TPOptimumLinearFIR::usage() {
  printf("(-tp ol)   -ol-m   Sets filter order (length)\n");
  printf("(-tp ol)   -ol-h   Sets filter number of training outputs\n");
  printf("(-tp ol)   -ol-l   Sets forgetting factor\n");
  printf("(-tp ol)   -ol-r   Sets recursive to true\n");
}

//  /* Return range in which next sample would reside with high probability */
//Interval TPOptimumLinearFIR::getExpInterval() {
//  if (num_samples == 0)
//    return Interval();
//  double a = dX.min();
//  double b = dX.max();
//  double min = computeExpMin(a, b);
//  double max = computeExpMax(a, b);
//  Interval result = Interval(last_exp_value + min, last_exp_value + max);
//  int num = 0;
//  for (int i = 0; i < num_samples; i++)
//    if (Interval(min, max).contains(dX(i)))
//      num++;
//  Logger::debugLog("Computed prediction range [%g,%g]/[%g,%g] with prob: %g\n",
//		   a, b, result.getMin(), result.getMax(), num / (double) num_samples);
//  return result;
//}

// Interval TPOptimumLinearFIR::getExpInterval() {
//   double c_mean = getExpValue();
//   if (c_mean == -1)
//     return Interval();
//   double dev_neg = mm.getDevNegValue();
//   double dev_pos = mm.getDevPosValue();
//   double c_min;
//   if (dev_neg == -1)
//     c_min = c_mean * 0.75;
//   else
//     c_min = c_mean + 1.0 * dev_neg;
//   double c_max;
//   if (dev_pos == -1)
//     c_max = c_mean * 1.25;
//   else
//     c_max = c_mean + 1.0 * dev_pos;

//   Interval iv(c_min, c_max);

//   if (num_samples > 0) {
//     int correct_preds = 0;
//     for (int i = 0; i < num_samples; i++)
//       if (iv.contains(X(i)))
// 	  correct_preds++;
//     Logger::debugLog("Returning interval [%g,%g] (alpha=%g) with prediction rate: %02.2f%%\n",
// 		     c_min, c_max, c_min / c_max, correct_preds * 100.0 / num_samples);
//     Logger::debugLog("Worstcase interval [%g,%g] (alpha=%g) with prediction rate 100%%\n",
// 		     X.extract(0, num_samples-1).min(), X.extract(0, num_samples-1).max(), X.min() / X.max());
//   }
//   return iv;
// }

static ColumnVector reverse(ColumnVector v) {
  int n = v.length();
  ColumnVector r(n);
  for (int i=0; i<n; i++)
    r(n-1-i) = v(i);
  return r;
}

static void shiftBackward(ColumnVector& v, double val) {
  int n = v.length();
  for (int i=1; i<n; i++)
    v(i-1) = v(i);
  v(n-1) = val;
}

  /* Return expected next value */
double TPOptimumLinearFIR::getExpValue() const {
  if (num_samples == 0)
    return -1;
  else if (num_samples < M+H) {
    double sum = 0.0;
    for (int i=0; i<num_samples; i++)
      sum += X(i);
    last_exp_value = sum / num_samples;
  } else
    last_exp_value = W.transpose() * reverse(X.extract(H, H+M-1));
  return last_exp_value;
}

void TPOptimumLinearFIR::addSample(double sample) {
  Logger::debugLog("num_samples = %d\n", num_samples);
  if (num_samples == 0) {
    X.resize(M+H);
    dX.resize(M+H);
  }
  if (num_samples < M+H) {
    X(num_samples) = sample;
    dX(num_samples) = sample - last_exp_value;
  } else {
    shiftBackward(X, sample);
    shiftBackward(dX, sample - last_exp_value);
    ASSERT(X.length() == M+H, "This is a problem !");
  }

  if ((num_samples == M+H-1) || (recursive && (num_samples == M+H))) {
    /* Compute optimum filter */
    Logger::debugLog("Training filter: building matrix...\n");
    Matrix FI(H, M);
    ColumnVector Y(H);
    for (int k = M-1; k < X.length()-1; k++) {
      RowVector temp(M);
      for (int i=0; i<M; i++)
	temp(i) = X(k-i);
      //      cout << FI;
      //      cout << Y;
      FI.insert(temp, k-(M-1), 0);
      Y(k-(M-1)) = X(k+1);
    }
    Logger::debugLog("Training filter: solving optimum...\n");
    W = (FI.transpose()*FI).inverse() * FI.transpose() * Y;
    Logger::debugLog("Training filter: ... done\n");
  }

  if (num_samples < M+H)
    num_samples++;

  mm.addSample(sample);
}
