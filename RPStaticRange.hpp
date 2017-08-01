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

#ifndef __ARSIM_RP_STATIC_RANGE_HPP__
#  define __ARSIM_RP_STATIC_RANGE_HPP__

#include "DoubleRangePredictor.hpp"

#include <deque>

using namespace std;

class RPStaticRange : public DoubleRangePredictor {

 private:

  /** All parameters are configured statically through command line
   * options
   */

  double c_min;		// Absolute minimum
  double c_max;		// Absolute maximum
  double c_min_i;	// High probability lower bound
  double c_max_i;	// High probability upper bound

public:

  static void usage();

  RPStaticRange();
  virtual bool parseArg(int& argc, char **& argv);

  /** Add a c_k sample */
  virtual void addSample(double sample);
  /** Return range in which next sample would reside with high probability */
  virtual Interval getExpInterval();
  /** Return range in which next sample would reside with lower probability */
  virtual Interval getExpIntervalI();
};

#endif
