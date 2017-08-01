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

#ifndef __ARSIM_SPIKE_TASK_HPP__
#  define __ARSIM_SPIKE_TASK_HPP__

#include "DoubleLimitedTask.hpp"
#include "Stat.hpp"

class SpikeTask : public DoubleLimitedTask {

 protected:

  typedef DoubleLimitedTask parent;

  double pr_full_range;	/** Probability of full range excursion	*/

 public:

  SpikeTask();
  /** Return next task instance time c(k) */
  double generateInstance();
  bool parseArg(int& argc, char **& argv);

  static void usage();
};

#endif
