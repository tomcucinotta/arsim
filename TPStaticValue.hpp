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

#ifndef __ARSIM_TP_STATIC_PARAMS_HPP__
#  define __ARSIM_TP_STATIC_PARAMS_HPP__

#include "ValuePredictor.hpp"

#include <deque>

using namespace std;

class TPStaticValue : public ValuePredictor {

 private:

  /** All parameters are configured statically through command line
   * options
   */

  double c_mean;		// Mean c_k value

public:

  static void usage();

  TPStaticValue();
  virtual bool parseArg(int& argc, char **& argv);

  /** Add a c_k sample */
  virtual void addSample(double sample);
  /** Return expected next value */
  virtual double getExpValue() const;
};

#endif
