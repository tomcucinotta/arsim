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

#ifndef __ARSIM_CMDLINE_TASK_HPP__
#  define __ARSIM_CMDLINE_TASK_HPP__

#include "Task.hpp"

#include <vector>

/** A task whose samples are specified via command-line options */

class CmdlineTask : public Task {

 protected:

  typedef Task parent;

  std::vector<double> samples;
  std::vector<double>::iterator it;

 public:

  CmdlineTask();
  /** Return next task instance time c(k) */
  double generateInstance();
  bool parseArg(int& argc, char **& argv);

  static void usage();
};

#endif
