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

#ifndef _TP_OPT_LIN_FIR_H_
#  define _TP_OPT_LIN_FIR_H_

#include "ValuePredictor.hpp"
#include "TPMoveableMean.hpp"
#include <octave/oct.h>

#define MAX_FIR_SIZE 256

using namespace std;

class TPOptimumLinearFIR : public ValuePredictor {

protected:

  ColumnVector W;	// The filter coefficients
  int M;		// The filter size == W.length()
  int H;		// The number of predictions on which to optimize
  ColumnVector X;	// Old samples
  ColumnVector dX;	// Errors performed by getExpValue()
  double lambda;	// The aging factor
  int num_samples;	// Used during initialization
  bool recursive;	// Whether or not re-train filter at each step

  mutable double last_exp_value; // Last value output by getExpValue()

  TPMoveableMean mm;	// Estimates output deviation from predicted value

public:

  typedef ValuePredictor parent;

  static void usage();

  TPOptimumLinearFIR();
  virtual bool parseArg(int& argc, char **& argv);

  /** Add a c_k sample */
  virtual void addSample(double sample);

  /** Return expected next value */
  virtual double getExpValue() const;

  double computeExpMin(double a, double b);
  double computeExpMax(double a, double b);
};

#endif
