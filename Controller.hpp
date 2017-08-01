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

#ifndef __ARSIM_CONTROLLER_HPP__
#  define __ARSIM_CONTROLLER_HPP__

#include "Task.hpp"
#include "Stat.hpp"
#include "Interval.hpp"
#include "TaskPredictor.hpp"

#include <stdio.h>
#include <deque>

/**
 * @file
 *
 * @brief Base class for QoS feedback controllers
 */

class ResourceManager;
class TaskScheduler;

/**
 * Base class for QoS feedback controllers.
 *
 * This class is also used for instantiating a new controller through
 * the getInstance() static method.
 */
class Controller : public Component {

 private:

  Task *p_task;
  TaskPredictor *p_tpred; //< For predicting/providing c(k) statistics related information

 protected:

  int curr_k;		//< Current time (task activation)
  double c_prev;	//< Previous task instance duration: c(k-1)
  double bw_prev_iot;   //< Last computed bandwidth if on time
  double bw_max;	//< Maximum available bandwidth for this controller
  double bw_min;	//< Minimum guaranteed bandwidth for this controller

  bool dyn_bw_active;	//< If set, dynamically maximizes bandwidth right after deadline violation
  double dyn_bw_rel_dl;	//< Percentage of period at which scheduler switches to max bandwidth

  Stat pe_stat;		//< Statistics of prediction error
  Stat pr_stat;		//< Statistics of correct prediction range

  FILE *pred_file;	//< Trace of predictor-related info

  int num_rs;		//< Number of resource
  int num_task;		//< Number of task on this resource

 public:

  /** Create a new Controller based on name */
  static Controller * getInstance(const char *s);

  Controller();
  virtual ~Controller();
  virtual bool parseArg(int& argc, char **& argv);
  /** Calculate b(k) for next task instance */
  virtual double calcBandwidth(double sched_err, double start_err) {
    return bw_max;
  }
  virtual double getBandwidthIfOnTime() const {
    return bw_prev_iot;
  }
  /** Check consistency of provided and default parameters */
  virtual bool checkParams();
  /** Provide the feedback information to the controller / predictor	*/
  virtual void setLastJobExecTime(double c);

  /** Set task */
  virtual void setTask(Task* p_t);
  /** Get task */
  virtual Task* getTask() const { return p_task; }

  /** Calculate further parameters, if any */
  virtual void calcParams();
  /** Dump statistics at end of simulation      **/
  virtual void dumpStats();
  /** Get last measured execution time          **/
  double getTaskTime() const { return c_prev; }
  /** Get maximum bandwidth available for this controller **/
  double getMaxBandwidth() const { return bw_max; }
  /** Get minimum bandwidth guaranteed to this controller **/
  double getMinBandwidth() const { return bw_min; }
  /** Set minimum bandwidth guaranteed to this controller **/
  void setMinBandwidth(double bw) { bw_min = bw; }

  /** Get average value of sequence of job execution times      **/
  double getTaskExpValue() const;
  /** Get standard deviation of sequence of job execution times **/
  double getTaskDev();
  double getTaskDevPos();
  double getTaskDevNeg();
  /** Get expected interval for next execution time, as provided by predictor **/
  Interval getTaskExpInterval() const;

  void setResourceId(int num_rs) { this->num_rs = num_rs; }
  void setControllerId(int num_task) { this->num_task = num_task; }

  /** Dump usage message on stdout              **/
  static void usage();

  TaskPredictor *getTaskPredictor() { return p_tpred; }

  virtual void clearHistory() { p_tpred->clearHistory(); }
};

#endif /* __ARSIM_CONTROLLER_HPP__ */
