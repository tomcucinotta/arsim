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

#ifndef __ARSIM_TRIANGLE_TASK_HPP__
#  define __ARSIM_TRIANGLE_TASK_HPP__

#include "Task.hpp"

class TriangleTask : public Task {

 private:

  double c_last, dc_last;

 public:

  TriangleTask();
  void setParams(double min_c, double max_c);
  /** Return next task instance time c(k) */
  double generateInstance();

  static void usage();
};

#endif
