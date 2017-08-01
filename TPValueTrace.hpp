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

#ifndef _TP_RANGE_TRACE_H_
#  define _TP_RANGE_TRACE_H_

#include "ValuePredictor.hpp"

#include <stdio.h>
#include <vector>

class TPValueTrace : public ValuePredictor {

protected:

  typedef ValuePredictor parent;

  char *trace_fname;
  double mul_factor;    //< Multiply factor to use for all read samples
  int disc_lines;       //< Lines of the trace file to discard from the head
  int col_number;       //< Column number to be used from the trace file
  std::vector<double> samples;
  std::vector<double>::iterator curr_sample;

public:

  static void usage();

  TPValueTrace();
  virtual ~TPValueTrace();
  virtual bool parseArg(int& argc, char **& argv);
  virtual void calcParams();

  /** Add a c_k sample */
  virtual void addSample(double sample);
  /** Return expected next value */
  virtual double getExpValue() const;
};

#endif
