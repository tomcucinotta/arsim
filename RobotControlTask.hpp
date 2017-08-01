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

#ifndef __ARSIM_ROBOT_CONTROL_TASK_HPP__
#  define __ARSIM_ROBOT_CONTROL_TASK_HPP__

#include "Task.hpp"
#include "Events.hpp"

class RobotControlTask : public Task {

 private:

  /* Add here your private members */

 public:

  RobotControlTask();

  /** Return next task instance time c(k) */
  double generateInstance();

  void handleSimStart(const Event & ev);

  /** Parse specific command-line arguments for this class	*/
  virtual bool parseArg(int& argc, char **& argv);

  /** Options for RobotControlTask, displayed to the user on -h */
  static void usage();
};

#endif
