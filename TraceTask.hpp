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

#ifndef __ARSIM_TRACE_TASK_HPP__
#  define __ARSIM_TRACE_TASK_HPP__

#include "DoubleLimitedTask.hpp"

#include <stdio.h>
#include <vector>

using namespace std;

class TraceTask : public DoubleLimitedTask {

 private:

  typedef DoubleLimitedTask parent;
  char *trace_fname;
  double mul_factor;    //< Multiply factor to use for all read samples
  int disc_lines;       //< Lines of the trace file to discard from the head
  int col_number;       //< Column number to be used from the trace file
  double sat_p_min;     //< Bottom percentile saturation value for the input samples
  double sat_p_max;     //< Top percentile saturation value for the input samples
  vector<double> samples;
  vector<double>::iterator curr_sample;

 public:

  TraceTask();
  ~TraceTask();

  /** Return next task instance time c(k) */
  double generateInstance();
  bool parseArg(int& argc, char **& argv);

  void loadTrace();

  static void usage();

  virtual bool checkParams();
  virtual void calcParams();

  vector<double> const & getSamples() const {
    return samples;
  }
};

#endif
