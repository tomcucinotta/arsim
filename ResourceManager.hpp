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

#ifndef __ARSIM_RESOURCE_MANAGER_HPP__
#  define __ARSIM_RESOURCE_MANAGER_HPP__

/** Project related interface includes	*/

#include "TaskScheduler.hpp"
#include "Events.hpp"
#include "Supervisor.hpp"
#include "LinearModel.hpp"

/** System includes			*/

#include <vector>
#include <string>

using namespace std;

class ResourceManager {

protected:

  vector<TaskScheduler*> tasks;	/**< Tasks to be scheduled	*/
  string rs_name;		/**< Resource name		*/
  static int next_rs_id;
  int rs_id;
  Supervisor *p_spv;		/**< The supervisor		*/
  vector<double> speeds;        /**< Speed corresponding to each power mode */
  int pow_mode;                 /**< Current power mode         */
  TimeStat *p_pow_mode_stats;   /**< Power mode statistics      */

public:

  static vector<ResourceManager*> rs_controllers;

  ResourceManager();

  static void usage();

  virtual bool parseArg(int& argc, char **& argv);

  void setResourceName(const char *rs_name);
  int getResourceId() const { return rs_id; }

  Supervisor *getSupervisor() const { return p_spv; }

  /** Handle the start of simulation event	*/
  virtual void handleSimStart(const Event & ev);

  /** Handle the arrive of a job event		*/
  virtual void handleJobArrive(const Event & ev);

  /** Handle the start of a job event		*/
  virtual void handleJobStart(const Event & ev);

  /** Handle the end of a job event		*/
  virtual void handleJobEnd(const Event & ev);

  /** Calculate further parameters, if any	*/
  static void calcParamsAll();

  /** Check params before simulation start	*/
  static bool checkParamsAll();

  /** Calculate further parameters, if any  */
  void calcParams();

  /** Check params before simulation start  */
  bool checkParams();

  /** Check and enforce global constraint	*/
  void checkGlobalConstraint(const Event & ev);

  /** Dump situation of tasks on various files	*/
  static void dump();

  /** Dump statistics of tasks on various files	*/
  static void dumpStatistics();

  /** Get the job ID of the last finished job	*/
  long getLastFinishedJobID(unsigned int num_task);

  /** Get the task scheduler by position		*/
  TaskScheduler *getTaskSchedulerAt(unsigned int tsk);

  /** Retrieve a task scheduler position		*/
  int getTaskSchedulerPos(TaskScheduler *p_tsched);

  /** Retrieve number of tasks in this resource		*/
  unsigned int getTaskSchedulerNum() const { return tasks.size(); }

  /** Set supervisor							*/
  void setSupervisor(Supervisor *p_spv);

  /** Get speed corresponding to supplied mode */
  double getSpeedForMode(int res_mode) const;

  /** Set current power mode */
  void setPowerMode(int mode);

  /** Get current power mode */
  int getPowerMode() const;

  TimeStat const & getPowerModeStatistics() const { return *p_pow_mode_stats; }

  /** Destructor					*/
  virtual ~ResourceManager();
};

/** Current resource controller, used during parseArg() calls	*/
extern ResourceManager *p_gsched;

#endif
