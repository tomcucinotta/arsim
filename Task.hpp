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

#ifndef _TASK_H_
#  define _TASK_H_

#include "Component.hpp"
#include "LinearModel.hpp"
#include "TimeStat.hpp"
#include "util.hpp"

#include <stdlib.h>
#include <vector>

class Task : public Component {

 protected:

  double period;
  double c_min, c_max;

  static double getRandom() { return double(random())/double(RAND_MAX); }

  static int next_task_id;

  int task_id;

  /** Current application mode (0 is the most powerful) **/
  unsigned int app_mode;
  unsigned int prev_app_mode;
  TimeStat *p_app_mode_stats;

  /** For each app_mode, the corresponding linear model (first one should be m=1.0, q=0.0) */
  std::vector<LinearModel*> app_mode_models;

  /** Return next task instance time c(k), under the assumption of application mode 0   */
  virtual double generateInstance();

 public:

  static Task * getInstance(const char * s);
  Task();
  virtual ~Task();
  int getId() const { return task_id; }
  /** Parse task-specific command-line arguments	*/
  virtual bool parseArg(int& argc, char **& argv);
  /** Set min and max instance time and T	*/
  virtual void setParams(double T, double min_c, double max_c);
  /** Return next task instance time c(k) corresponding to the current application mode */
  virtual double generateWorkingTime();
  /** Return task period				*/
  double getPeriod() const { return period; }
  /** Return min task instance execution time	*/
  double getMinExecutionTime() const { return c_min; }
  /** Return max task instance execution time	*/
  double getMaxExecutionTime() const { return c_max; }

  static void usage();

  virtual bool checkParams() {   Logger::debugLog("Checking task params\n");  return true;  }
  virtual void calcParams() {   Logger::debugLog("Computing task params\n");  }

  virtual void setAppMode(unsigned int mode);
  virtual unsigned int getAppMode() const { return app_mode; }
  virtual unsigned int getPreviousAppMode() const { return prev_app_mode; }
  TimeStat const & getAppModeStatistics() const { return *p_app_mode_stats; }

  LinearModel & getModelForMode(unsigned int app_mode);
};

#endif
