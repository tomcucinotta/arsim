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

#ifndef __ARSIM_TASK_SCHEDULER_HPP__
#  define __ARSIM_TASK_SCHEDULER_HPP__

#include "Task.hpp"
#include "Controller.hpp"
#include "Events.hpp"
#include "TimeStat.hpp"
#include "RPStatBased.hpp"

#include <queue>
#include <string>

//using namespace std;

class ResourceManager;

class TaskScheduler : public Component {

private:

  Controller *p_sched;
  ResourceManager *p_gsched;

  Event *p_job_arrive, *p_job_start, *p_job_end, *p_bw_change;

  /** Emulate the job queue: queue length	*/
  int num_jobs;

  /** Bandwidth required by the local scheduler	*/
  double bw_required;

  /** Used with ceil model: new bw set by global
      scheduler to be enforced on next server period */
  double bw_current_new;

  /** Current bandwidth set by global scheduler	*/
  double bw_current;

  /** Job ID, i.e. number of current job:1,2,..	*/
  long curr_job_id;

  /** Last finished Job ID, or 0 if none yet.	*/
  long last_job_id;

  /** Current job execution time (if bw==1)	*/
  Time c_current_total;
  /** Current job residual execution time (i.e. the current budget)	*/
  Time c_current_left;
  /** Time at which the residual execution time
   * was equal to c_current_left		*/
  Time t_start;

  /** Last measured scheduling error value	*/
  Time sched_err;

  /** If set, enables ceil() model for e(k)	*/
  bool ceil_model;
  /** Integer eps(k), used for ceil() model	*/
  long i_eps;
  /** Server period, used with ceil model	*/
  Time server_period;

  /** Time of first instance arrive (may be delayed if pipelined task)	*/
  Time start_time_offset;

  /** Count the number of class instances	*/
  static int num_tasks;

  /** File Name for All events trace */
  char *fname;
  /** File for All events trace */
  FILE *out_file;
  bool out_file_opened;

  /** File for scheduling error trace */
  FILE *se_trace_file;

  /** The temporal statistics for the bandwidth	*/
  TimeStat bw_time_stat;	// Current
  TimeStat rbw_time_stat;	// Required

  /** Statistics on required bandwidth since rbw_avg_time                 */
  TimeStat rbw_avg_time_stat;
  /** Last time at which the rbw_avg_time_stat statistics have been reset */
  Time rbw_avg_time;

  /** Time statistics for required-assigned bw	*/
  TimeStat dbw_time_stat;

  /** The event-based statistics for sched err	*/
  Stat *se_stat;

  /** The statistics for c_k			*/
  Stat *ck_stat;

  /** The percentile estimator for c_k, reset at each optimization period */
  RPStatBased ck_perc_est_temp;

  /** event-based statistics for number of consecutive jobs for which the sched err remains positive */
  Stat rsteps_stat;

  /** Partial number of consecutive jobs for which the sched err remains positive */
  int rsteps;

  double inv_e, inv_E;	//< Probability of s.e. inside this set always computed by emulator */
  double inv_ei, inv_Ei;//< Probability of s.e. inside this set always computed by emulator */
  Stat pinv_stat;	//< Statistics of invariant respected */
  Stat pinvi_stat;      //< Statistics of invariant respected */
  Stat pinv_stat_temp;  //< Statistics of invariant respected (temporary, resets at each optimization period) */

  /** Also updates statistics consistently	*/
  void setCurrentBandwidth(double b);
  /** Also updates statistics consistently	*/
  void setRequiredBandwidth(double b);

  /** Also updates statistics and delta bw consistently	*/
  void setCurrentBandwidthDelta(double b);
  /** Also updates statistics and delta bw consistently	*/
  void setRequiredBandwidthDelta(double b);

  /** Previous tasks in the pipeline			*/
  vector<TaskScheduler *> pl_prev;
  /** Subsequent tasks in the pipeline			*/
  vector<TaskScheduler *> pl_next;
  /** True iff this task is blocked due to previous jobs in pipeline */
  bool pl_blocked;

  /** String used to refer to tasks that depend on this	*/
  char *pl_next_str;
  /** String used to refer to tasks this tas depends on	*/
  char *pl_prev_str;

  /** Log Scheduling Error to File */
  void logSchedErrTrace(double err);

  /** Weight used for weighted fair bandwidth distribution algorithms */
  double weight;

  void updateRequiredBandwidthAvg();

public:

  TaskScheduler(Controller *p_s, ResourceManager *p_gs);

  static void usage();

  virtual bool parseArg(int & argc, char ** & argv);

  /** Handle the start of simulation event	*/
  virtual void handleSimStart(const Event & ev);
  /** Handle event: arrive of a job		*/
  virtual void handleJobArrive(const Event & ev);
  /** Handle event: start of a job		*/
  void handleJobStart(const Event & ev);
  /** Handle event: end of a job		*/
  virtual void handleJobEnd(const Event & ev);
  /** Handle event: actually apply a bw change	*/
  void handleBwChange(const Event& ev);
  /** Destructor				*/
  virtual ~TaskScheduler();

  void addEventJobArrive(Time delta_t);
  void addEventJobStart(Time delta_t);
  void addEventJobEnd(Time c, double b);

  Task *getTask() { return p_sched->getTask(); }
  Controller *getController() { return p_sched; }
  double getRequiredBandwidth() const { return bw_required; }
  double getCurrentBandwidth() const { return bw_current; }
  double getWeight() const { return weight; }

  double getRequiredBandwidthAvg();
  void clearRequiredBandwidthAvg();
  /** Forget historical workload data */
  void clearHistory();

  double getMeanBandwidth() const { return bw_time_stat.getMean(); }
  double getMeanRequiredBandwidth() const { return rbw_time_stat.getMean(); }
  double getMeanDeltaBandwidth() const { return dbw_time_stat.getMean(); }
  double getMeanSchedError() const { return se_stat->getMean(); }
  Stat & getTempPDNVStat() { return pinv_stat_temp; }

  /** This also manages job end and bw change events	*/
  void changeCurrentBandwidth(double b);

  /** Actually apply a bw change required by global
   * controller through a changeCurrentBandwidth()	*/
  void changeCurrentBandwidthNow();

  /** Return the last scheduling error value	*/
  double getSchedError() const { return sched_err; }

  /** Return the last finished-job ID		*/
  long getLastFinishedJobID() const { return last_job_id; }

  /** Dump status information on out_file	*/
  void dump();
  /** Dump statistics to proper files		*/
  void dumpStatistics();

  static void dumpStat(const BaseStat & time_stat,
		       const char *fname, const char *var_name,
		       bool dump_as_pmf = false,
		       const char *comment = "No comment");

  /** Say if a job is running for the task	*/
  bool isRunning();

  /** Add to the previous tasks in pipeline	*/
  void addPipelinePrev(TaskScheduler *p_tsched);

  /** Add to the next tasks in pipeline		*/
  void addPipelineNext(TaskScheduler *p_tsched);

  /** Say if all jobs preceeding next_job_id in pipeline terminated */
  bool checkPipelinePrevJobs(int next_job_id);

  /** Say if this task is blocked on others	*/
  bool isBlocked();

  /**
   * Notify that the supplied job, which preceeds this task in pipeline,
   * has terminated a job.
   *
   * If this task was not blocked, this call is ignored. If it was blocked,
   * this call causes re-evaluation of the blocking condition and possible
   * start of the new job if all previous jobs have already terminated.
   */
  void notifyPipelinePrevJobEnd(TaskScheduler *p_tsched);

  void setResourceManager(ResourceManager *p) { p_gsched = p; }
  ResourceManager *getResourceManager() const { return p_gsched; }

  Time getStartTime() const;

  virtual bool checkParams();
};

#endif
