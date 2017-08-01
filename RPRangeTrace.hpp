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

#ifndef _RP_RANGE_TRACE_H_
#  define _RP_RANGE_TRACE_H_

#include "RangePredictor.hpp"

#include <stdio.h>

using namespace std;

class RPRangeTrace : public RangePredictor {

 protected:

  char *trace_fname;
  FILE *ff;
  int num_samples;	/** Just counting how many samples                */
  Interval curr_iv;	/** Current interval, used for statistics getters */

  void nextInterval();

public:

  static void usage();

  RPRangeTrace();
  virtual ~RPRangeTrace();
  virtual bool parseArg(int& argc, char **& argv);

  /** Add a c_k sample */
  virtual void addSample(double sample);
  /** Return range in which next sample would reside with high probability */
  virtual Interval getExpInterval();
};

#endif
