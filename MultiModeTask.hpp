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

#ifndef __ARSIM_MULTIMODE_TASK_HPP__
#  define __ARSIM_MULTIMODE_TASK_HPP__

#include "Task.hpp"

/** Multi-mode task: supports definition of multiple modes of operation,
 ** each of them represented through an instance of a subclass of Task,
 ** with its own parameters configuration.
 **/
class MultiModeTask : public Task {

 private:

  std::vector<Task *> app_mode_tasks;
  /** Overrides parent-class app_mode variable */
  unsigned int mmt_app_mode;
  unsigned int prev_mmt_app_mode;

 public:

  typedef Task parent;
  MultiModeTask();
  /** Return next task instance time c(k). Actually, probes ALL defined app modes,
   ** but returns only the value associated with the current application mode.
   **/
  virtual double generateWorkingTime();

  virtual void setAppMode(unsigned int mode);
  virtual unsigned int getAppMode() const { return mmt_app_mode; }
  virtual unsigned int getPreviousAppMode() const { return prev_mmt_app_mode; }

  virtual bool parseArg(int& argc, char **& argv);
  static void usage();

  virtual bool checkParams();
  virtual void calcParams();
};

#endif
