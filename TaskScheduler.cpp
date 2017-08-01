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

#include <cstring>

#include "TaskFactory.hpp"
#include "TaskScheduler.hpp"
#include "ResourceManager.hpp"
#include "defaults.hpp"
#include "TaskPredictor.hpp"
#include "GlobalOptimizer.hpp"

#include <sstream>

TaskScheduler::TaskScheduler(Controller *p_s, ResourceManager *p_gs)
: bw_time_stat(0.0, 1.0, 0.01),
rbw_time_stat(0.0, 1.0, 0.01),
rbw_avg_time_stat(0.0, 1.0, 0.01),
dbw_time_stat(0.0, 1.0, 0.01),
rsteps_stat(0.0, 10.0, 1.0),
pinv_stat(0.0, 2.0, 1.0),
pinvi_stat(0.0, 2.0, 1.0),
pinv_stat_temp(0.0, 2.0, 1.0)
{
  p_sched = p_s;
  p_gsched = p_gs;

  p_job_arrive = p_job_end = p_job_start = 0;
  c_current_left = 0.0;
  curr_job_id = 0;
  last_job_id = 0; // No job finished yet.
  sched_err = 0.0;
  rsteps = 0;

  ceil_model = false;
  i_eps = 0;
  server_period = DEF_SRV_PER;
  p_bw_change = 0;

  num_jobs = 0;
  int num_rs = getResourceManager()->getResourceId();
  // New TaskScheduler not registered yet into the ResourceManager
  int num_task = getResourceManager()->getTaskSchedulerNum();

  fname = strdup("task0,0.dat");
  fname[4] = '0' + num_task;
  fname[6] = '0' + num_rs;
  /* Delay file open. parseArg() could change file name	*/
  out_file_opened = false;

  setRequiredBandwidth(0.0);
  setCurrentBandwidthDelta(0.0);

  pl_blocked = false;

  se_stat = 0; /**< Allow for command line sched err statistics customization	*/
  ck_stat = 0; /**< Allow for command line c_k statistics customization	*/

  pl_next_str = pl_prev_str = 0;

  inv_e = 0;
  inv_E = 0;

  se_trace_file = 0;

  weight = 1.0;
}

void TaskScheduler::usage() {
  printf("           -pa t[,r] Pipe task after specified task\n");
  printf("           -ceil   Adopt ceil model for scheduling error evolution\n");
  printf("           -P      Set server period (requires -ceil)\n");
  printf("           -inv-e  Set virtual invariant for statistics\n");
  printf("           -inv-E  Set virtual invariant for statistics\n");
  printf("           -inv-ei Set virtual 2nd invariant for statistics\n");
  printf("           -inv-Ei Set virtual 2nd invariant for statistics\n");
  printf("           -w      Set weight for bandwidth distribution\n");
  printf("           -t      Set task type: u/s/t/p/tp/tr/cl\n");
  Task::usage();
  TaskFactory::usage();
  Controller::usage();
}

bool TaskScheduler::parseArg(int & argc, char ** & argv) {
  if (strcmp(*argv, "-pa") == 0) {
    CHECK(argc> 1, "Option requires an argument");
    argv++;
    argc--;
    ResourceManager *p_gs = p_gsched;
    unsigned int tsk, rs;
    if (sscanf(*argv, "%u,%u", &tsk, &rs) == 2) {
      CHECK(rs < ResourceManager::rs_controllers.size(),
          "Pipelined task may only depend on tasks within already defined resources, sorry.");
      p_gs = ResourceManager::rs_controllers[rs];
    } else
      CHECK(sscanf(*argv, "%u", &tsk) == 1, "Wrong argument to -pa");

    TaskScheduler *p_tsched = p_gs->getTaskSchedulerAt(tsk);
    CHECK(p_tsched != 0, "Pipelined task may only depend on already defined tasks, sorry.");
    CHECK(p_tsched != this, "Pipelined task cannot depend on itself. Maybe forgot resource spec ?");

    addPipelinePrev(p_tsched);
    p_tsched->addPipelineNext(this);
  } else if (strcmp(*argv, "-ceil") == 0) {
    ceil_model = true;
  } else if (strcmp(*argv, "-P") == 0) {
    CHECK(argc> 1, "Option requires an argument");
    argv++;
    argc--;
    CHECK(sscanf(*argv, "%lg", &server_period) == 1, "Expecting double as argument to -P option");
  } else if (strcmp(*argv, "-inv-e") == 0) {
    CHECK(argc> 1, "Option requires an argument");
    argv++;
    argc--;
    CHECK(sscanf(*argv, "%lg", &inv_e) == 1, "Expecting double as argument to -inv-e option");
    CHECK(inv_e >= 0.0, "Expecting double in the range [0,+oo[ as argument to -inv-e option");
  } else if (strcmp(*argv, "-inv-E") == 0) {
    CHECK(argc> 1, "Option requires an argument");
    argv++;
    argc--;
    CHECK(sscanf(*argv, "%lg", &inv_E) == 1, "Expecting double as argument to -inv-E option");
    CHECK(inv_E >= 0.0, "Expecting double in the range [0,+oo[ as argument to -inv-E option");
  } else if (strcmp(*argv, "-inv-ei") == 0) {
    CHECK(argc> 1, "Option requires an argument");
    argv++;
    argc--;
    CHECK(sscanf(*argv, "%lg", &inv_ei) == 1, "Expecting double as argument to -inv-ei option");
    CHECK(inv_ei >= 0.0, "Expecting double in the range [0,+oo[ as argument to -inv-ei option");
  } else if (strcmp(*argv, "-inv-Ei") == 0) {
    CHECK(argc> 1, "Option requires an argument");
    argv++;
    argc--;
    CHECK(sscanf(*argv, "%lg", &inv_Ei) == 1, "Expecting double as argument to -inv-Ei option");
    CHECK(inv_Ei >= 0.0, "Expecting double in the range [0,+oo[ as argument to -inv-Ei option");
  } else if (strcmp(*argv, "-w") == 0) {
    CHECK(argc> 1, "Option requires an argument");
    argv++;
    argc--;
    CHECK(sscanf(*argv, "%lg", &weight) == 1, "Expecting double as argument to -w option");
    CHECK(weight> 0.0, "Expecting strictly positive real as argument to -w option");
  } else if (strcmp(*argv, "-t") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    Task * p_new_task = TaskFactory::getInstance(*argv);
    CHECK(p_new_task != 0, "Unknown task type");
    p_sched->setTask(p_new_task);
  } else
    return p_sched->parseArg(argc, argv);
  return true;
}

/** @todo Add support for dependent tasks on same or other resources	*/
void TaskScheduler::addEventJobArrive(Time delta_t) {
  Logger::debugLog("# T=%g: adding job arrive at %g\n", EventList::getTime(),
      EventList::getTime() + delta_t);
  p_job_arrive = makeEvent(delta_t, p_gsched,
			   &ResourceManager::handleJobArrive, this);
  ASSERT(p_job_arrive != 0, "No more memory");
  EventList::events().insert(p_job_arrive);
}

void TaskScheduler::addEventJobStart(Time delta_t) {
  Logger::debugLog("# T=%g: adding job start at %g\n", EventList::getTime(),
      EventList::getTime() + delta_t);
  p_job_start = makeEvent(0, p_gsched, &ResourceManager::handleJobStart, this);
  ASSERT(p_job_start != 0, "No more memory");
  EventList::events().insert(p_job_start);
}

/** Schedules a JobEnd event, given remaining computation time and currently used bandwidth.
 **
 ** The bandwidth here is specified as an absolute value, accounting also for resource speed.
 ** For example, b=0.5 means equivalently full utilization at half speed, or half utilization
 ** at full speed.
 **/
void TaskScheduler::addEventJobEnd(Time c, double b) {
  Time delta_t;
  if (!ceil_model)
    delta_t = c / b;
  else {
    Time q = b * server_period; /**< Max Budget	*/
    delta_t = ceil(c / q) * server_period;
  }
  Logger::debugLog("# T=%g: adding job end at %g\n", EventList::getTime(),
      EventList::getTime() + delta_t);
  p_job_end
      = makeEvent(delta_t, p_gsched, &ResourceManager::handleJobEnd, this);
  ASSERT(p_job_end != 0, "No more memory");
  EventList::events().insert(p_job_end);
}

/** Add eps stats customization through command line options */
void TaskScheduler::handleSimStart(const Event & ev) {
  CHECK(getTask() != 0, "Controller without tasks. Use the -t option, after -s.\n");
  Logger::debugLog("# T=%g: handling sim start\n", EventList::getTime());

  if (!ceil_model) {
    se_stat = new Stat(DEF_EPS_MIN, DEF_EPS_MAX, DEF_EPS_DX);
  } else {
    /** Ceil model: account eps values as multiples of the server period */
    double T = getTask()->getPeriod();
    double dx = server_period / T;
    double se_min = ((long) ((DEF_EPS_MIN - dx / 2) / dx)) * dx;
    double se_max = ((long) ((DEF_EPS_MAX + dx / 2) / dx)) * dx;
    Logger::debugLog("Creating Stat with: [%g:%g:%g]", se_min, dx, se_max);
    se_stat = new Stat(se_min, se_max, dx);
  }
  ASSERT(se_stat != 0, "Could not allocate Stat object !");

  ck_stat = new Stat(0.0, getTask()->getMaxExecutionTime() / getTask()->getPeriod(), 0.001);
  ASSERT(ck_stat != 0, "Could not allocate Stat object !");
  fprintf(stderr, "# ck_stat size = %ld (max c_k=%g)\n", ck_stat->getPMFSize(), getTask()->getMaxExecutionTime());

  if (pl_prev.size() == 0)
    addEventJobArrive(0);
  else
    Logger::debugLog("# Start of pipelined task: delaying first activation\n");
}

void TaskScheduler::handleJobArrive(const Event & ev) {
  ASSERT(getTask() != 0, "Null task");
  Logger::debugLog("# T=%g: handling job arrive\n", EventList::getTime());
  num_jobs++;
  if (num_jobs == 1) {
    /* Previous job already finished. Check previous jobs in pipeline	*/
    if (checkPipelinePrevJobs(curr_job_id+1)) {
      /* All previous jobs finished. Start immediately			*/
      Logger::debugLog("STARTING TASK AT ARRIVAL\n");
      pl_blocked = false;
      addEventJobStart(0);
    } else {
      /* At least one previous job not finished yet. Start later	*/
      Logger::debugLog("BLOCKING TASK\n");
      pl_blocked = true;
    }
  } else {
    /* Previous job not yet finished. Start later			*/
  }
  /* Schedule next periodic job arrive	*/
  addEventJobArrive(getTask()->getPeriod());

  /* If at first job arrive, set start offset for the task		*/
  if (curr_job_id == 0) {
    start_time_offset = EventList::getTime();
  }
  /* Support for pipelines: allow cascaded tasks to start		*/
  if (curr_job_id == 1) {
    vector<TaskScheduler*>::iterator it = pl_next.begin();
    while (it != pl_next.end()) {
      Logger::debugLog("End of first instance of pipelined job: adding delayed first activation of dependent task\n");
      (*it)->addEventJobArrive(0);
      it++;
    }
  }
}

void TaskScheduler::updateRequiredBandwidthAvg() {
  double est_H_k;
  if ( ck_perc_est_temp.getExpInterval().isEmpty() )
    est_H_k = getTask()->getMaxExecutionTime();
  else
    est_H_k = ck_perc_est_temp.getExpInterval().getMax();
  // double rbw_iot = p_sched->getBandwidthIfOnTime();
  double rbw_iot = est_H_k / getTask()->getPeriod();
  rbw_avg_time_stat.addSample(rbw_iot, EventList::getTime());
  Logger::debugLog("Required bw (if on time [est_H_k/T=%g]): %g, Avg: %g, Max: %g (app_mode: %d, res_mode: %d)\n",
		   est_H_k / getTask()->getPeriod(), rbw_iot,
		   rbw_avg_time_stat.getMean(), rbw_avg_time_stat.getMax(),
		   getTask()->getAppMode(), getResourceManager()->getPowerMode());
}

/**
 * Handle the start of a Job, i.e. if it was enqueued, it is going to
 * start right now.
 *
 * @todo Verify computation of new bandwidth happens here, not at job end,
 *       because the actual instant at which the job is started may be delayed
 *       due to dependencies among tasks.
 */
void TaskScheduler::handleJobStart(const Event & ev) {
  Logger::debugLog("handleJobStart() starting\n");
  ASSERT(getTask() != 0, "Null task");
  t_start = EventList::getTime();
  Time start_err = t_start - getTask()->getPeriod() * curr_job_id
      - getStartTime();
  curr_job_id++;
  ASSERT(checkPipelinePrevJobs(curr_job_id), "Starting a pipeline-blocked task");
  pl_blocked = false;
  Logger::debugLog(
      "T %g: Starting job %02d,%d,%d: start_err %lg, sched_err %lg\n", t_start,
      curr_job_id, p_gsched->getTaskSchedulerPos(this),
      p_gsched->getResourceId(), start_err, sched_err);
  /* Compute next task instance					*/
  c_current_left = getTask()->generateWorkingTime();
  c_current_total = c_current_left;
  ck_stat->addSample(c_current_total / getTask()->getPeriod());
  ck_perc_est_temp.addSample(c_current_total);
  /* Supply sample to predictor in order to allow perfect prediction (if enabled for the predictor) */
  p_sched->getTaskPredictor()->setPerfectPrediction(c_current_total);
  /* Set required bandwidth (no delta update)			*/
  setRequiredBandwidth(p_sched->calcBandwidth(sched_err, start_err));
  updateRequiredBandwidthAvg();
  /* Assume no compression occurs: this will be checked by
   * ResourceManager each time					*/
  setCurrentBandwidthDelta(bw_required); // Also updates delta bw
  /* Add job end event						*/
  addEventJobEnd(c_current_left, bw_current);
}

void TaskScheduler::logSchedErrTrace(double err) {
  int num_rs = getResourceManager()->getResourceId();
  int num_task = getResourceManager()->getTaskSchedulerPos(this);

  if (se_trace_file == 0) {
    char *se_fname;
    se_fname = strdup("se0,0.dat");
    se_fname[2] = '0' + num_task;
    se_fname[4] = '0' + num_rs;
    se_trace_file = fopen(se_fname, "w");
    ASSERT(se_trace_file != 0, "Could not open file !");
    fprintf(se_trace_file, "# eps_k\n");
  }
  fprintf(se_trace_file, "%g\n", err);
}

void TaskScheduler::handleJobEnd(const Event & ev) {
  ASSERT(num_jobs> 0, "handleJobEnd() with no jobs");
  ASSERT(p_job_end != 0, "handleJobEnd() with no p_job_end");
  /* Notify predictor of occurred job execution time	*/
  p_sched->setLastJobExecTime(c_current_total);
  Logger::debugLog("# T=%g: Job %02d executed for %lg\n", EventList::getTime(),
      curr_job_id, c_current_total);
  sched_err = EventList::getTime() - getTask()->getPeriod() * curr_job_id
      - getStartTime();
  if (!ceil_model) {
    se_stat->addSample(sched_err / getTask()->getPeriod());
  } else {
    /**
     * Ceil model: sched err accounting is based on multiples of server_period, thus by
     * forcing eps stat values to be slightly higher than each server period we avoid little
     * imprecisions in its accounting (see above how sched_err is computed).
     */
    se_stat->addSample(sched_err / getTask()->getPeriod());
  }

  if (sched_err <= 0) {
    rsteps_stat.addSample(rsteps); // Avoid imprecisions in accounting
    rsteps = 0;
  } else
    rsteps++;

  if ((sched_err >= -inv_e) && (sched_err <= inv_E)) {
    pinv_stat.addSample(1.0);
    pinv_stat_temp.addSample(1.0);
  } else {
    pinv_stat.addSample(0.0);
    pinv_stat_temp.addSample(0.0);
  }

  if ((sched_err >= -inv_ei) && (sched_err <= inv_Ei))
    pinvi_stat.addSample(1.0);
  else
    pinvi_stat.addSample(0.0);

  logSchedErrTrace(sched_err);

  Logger::debugLog("# T=%g: handling job %02d end with eps=%g\n",
      EventList::getTime(), curr_job_id, sched_err);
  num_jobs--;
  p_job_end = 0;
  c_current_left = 0;
  last_job_id = curr_job_id;
  if (num_jobs > 0) {
    /* Other jobs enqueued: check preceeding tasks in pipeline	*/
    if (checkPipelinePrevJobs(curr_job_id+1)) {
      /* All preceeding jobs terminated: start next one immediately	*/
      Logger::debugLog("STARTING TASK AT END OF PREVIOUS ONE\n");
      addEventJobStart(0);
    } else {
      /* At least one preceeding job not yet terminated: block
       * this task. It will be woken up by notifyPipelinePrevJobEnd	*/
      Logger::debugLog("BLOCKING TASK\n");
      pl_blocked = true;
      /** TODO: save current required bw, set required bw to zero. When
       * unblocking task, restore required bw. Also, account for ceil_model
       * in such things, and remember that a bw change could have been scheduled */
    }
  } else {
    /* No other jobs enqueued: set current bandwidth to zero	*/
    setRequiredBandwidth(0.0); // No delta bw update
    setCurrentBandwidthDelta(0.0); // Also updates delta bw
  }

  /* Support for ceil model: if a bandwidth change requested by
   * supervisor was scheduled in the future (at most at the start of
   * next server period) then remove it: the task will re-set its
   * required bandwidth at the start of the next job anycase
   */
  if (p_bw_change != 0) {
    /** Bw Change was scheduled, but job finished anticipately */
    bool found = EventList::events().remove(p_bw_change);
    ASSERT(found, "changeCurrentBandwidth(): bw change event not found");
    p_bw_change = 0;
  }

  /* Support for pipelines: if there are dependent tasks, then
   * notify them of our job end. This may result in starting the
   * tasks if they were blocked waiting for this job end of this task.
   */
  vector<TaskScheduler*>::iterator it = pl_next.begin();
  for (; it != pl_next.end(); it++)
    (*it)->notifyPipelinePrevJobEnd(this);
}

void TaskScheduler::setCurrentBandwidthDelta(double b) {
  setCurrentBandwidth(b);
  dbw_time_stat.addSample(bw_required - bw_current, EventList::getTime());
}

void TaskScheduler::setCurrentBandwidth(double b) {
  bw_current = b;
  Logger::debugLog("Current bw:%g (required bw: %g)\n", bw_current, bw_required);
  bw_time_stat.addSample(bw_current, EventList::getTime());
}

void TaskScheduler::setRequiredBandwidthDelta(double b) {
  setRequiredBandwidth(b);
  dbw_time_stat.addSample(bw_required - bw_current, EventList::getTime());
}

void TaskScheduler::setRequiredBandwidth(double b) {
  bw_required = b;
  rbw_time_stat.addSample(bw_required, EventList::getTime());
}

void TaskScheduler::clearRequiredBandwidthAvg() {
  rbw_avg_time_stat.clear(EventList::getTime());
  Logger::debugLog("Cleared Avg bw\n");
  // Set initial required bandwidth in avg statistics to the current one instead of zero
  updateRequiredBandwidthAvg();
}

double TaskScheduler::getRequiredBandwidthAvg() {
  updateRequiredBandwidthAvg();
  return rbw_avg_time_stat.getMean();
    //ck_perc_est_temp.getExpInterval().getMax() / getTask()->getPeriod();

  // Unfortunately, the approach below is seriously affected by uncertainties
  // of the local predictors right after the clearHistory(), which cause the
  // controller to output either the minimum guaranteed bandwidth, or the
  // maximum available one for the controller.
  //return rbw_avg_time_stat.getMax();
}

void TaskScheduler::changeCurrentBandwidthNow() {
  double b = bw_current_new;
  if (bw_current == b) // TODO Define some tolerance ?
    return;
  ASSERT(getTask() != 0, "changeCurrentBandwidthNow(): Null task");
  ASSERT(p_job_end != 0, "changeCurrentBandwidthNow(): No job end scheduled");
  bool found = EventList::events().remove(p_job_end);
  ASSERT(found, "changeCurrentBandwidthNow(): job-end event not found");
  /* Account for the amount of consumed job execution time	*/
  Time t_curr = EventList::getTime();
  // Here bw_current already accounts for resource speed, so no correction is needed.
  c_current_left -= (t_curr - t_start) * bw_current;
  //ASSERT(c_current_left >= -0.0000001, "changeCurrentBandwidthNow(): c_current_left < 0");
  t_start = t_curr;
  setCurrentBandwidthDelta(b);
  addEventJobEnd(c_current_left, bw_current);
}

void TaskScheduler::changeCurrentBandwidth(double b) {
  if (b < 0.001)
    b = 0.001;
  bw_current_new = b;
  if (!ceil_model)
    changeCurrentBandwidthNow();
  else {
    /**
     * In case of ceil model, delay the actuation of bandwidth change to the
     * next server activation. This, in the real kernel, assures schedulability
     * of the other servers under any condition. Furthermore, check computation
     * of residual computation time of the job according to the ceil model.
     */
    if (p_bw_change != 0) {
      /* Bw Change already scheduled: just overwrite new value, no need to add event */
    } else {
      double consumed = fmod(EventList::getTime() - getStartTime(),
          server_period);
      /* Apply immediately bw change if on multiple of server period. Also, tolerate
       * approximations in the computation of the exact position within the current server period
       */
      if (consumed < 0.000001)
        changeCurrentBandwidthNow();
      else {
        double delta_t = server_period - consumed;
        if (delta_t < 0.0) /** Just in case ... */
          delta_t = 0.0;
        Logger::debugLog("# T=%g: adding actual bw change at %g\n",
            EventList::getTime(), EventList::getTime() + delta_t);
        p_bw_change = makeEvent(delta_t, this,
            &TaskScheduler::handleBwChange);
        ASSERT(p_bw_change != 0, "No more memory");
        EventList::events().insert(p_bw_change);
      }
    }
  }
}

void TaskScheduler::handleBwChange(const Event& ev) {
  p_bw_change = 0;
  changeCurrentBandwidthNow();
}

template<class T> std::string ToString(const T& val) {
  std::ostringstream strm;
  strm << val;
  return strm.str();
}

void TaskScheduler::dump() {
  if (!out_file_opened) {
    Logger::debugLog("# Opening output trace file %s\n", fname);
    out_file = fopen(fname, "w");
    ASSERT(out_file != 0, "Couldn't open output trace file for task");
    fprintf(out_file, "# %9s %11s %11s %11s %11s %11s %11s %11s %11s %11s\n",
        "time", "T", "N_k", "c_k", "b_k", "rb_k", "Bmin", "e_k", "pl_n", "pl_p");
    out_file_opened = true;

    vector<TaskScheduler *>::iterator it;
    std::ostringstream s_pl_next;
    for (it = pl_next.begin(); it != pl_next.end(); ++it)
      s_pl_next << " " << (*it)->getResourceManager()->getTaskSchedulerPos(*it)
          << "," << (*it)->getResourceManager()->getResourceId();
    pl_next_str = strdup(s_pl_next.str().c_str());
    std::ostringstream s_pl_prev;
    for (it = pl_prev.begin(); it != pl_prev.end(); ++it)
      s_pl_prev << " " << (*it)->getResourceManager()->getTaskSchedulerPos(*it)
          << "," << (*it)->getResourceManager()->getResourceId();
    pl_prev_str = strdup(s_pl_prev.str().c_str());
  }

  double bw_min = getController()->getMinBandwidth();
  fprintf(out_file,
      "%11.4f %11.4f %11d %11.5f %11.5f %11.5f %11.5f %11.5f %11s %11s\n",
      EventList::getTime(), getTask()->getPeriod(), num_jobs, c_current_left,
      bw_current, bw_required, bw_min, sched_err, pl_next_str, pl_prev_str);
}

void TaskScheduler::dumpStatistics() {
  int num_rs = getResourceManager()->getResourceId();
  int num_task = getResourceManager()->getTaskSchedulerPos(this);

  /** File where to dump cur bandwidth stats to	*/
  char *bw_fname;
  bw_fname = strdup("bw_stats0,0.dat");
  bw_fname[8] = '0' + num_task;
  bw_fname[10] = '0' + num_rs;
  bw_time_stat.dumpStat(bw_fname, "bw", false, "Granted bandwidth");

  /** File where to dump req bandwidth stats to	*/
  char *rbw_fname;
  rbw_fname = strdup("rbw_stats0,0.dat");
  rbw_fname[9] = '0' + num_task;
  rbw_fname[11] = '0' + num_rs;
  rbw_time_stat.dumpStat(rbw_fname, "rbw", false, "Required bandwidth");

  /** File where to dump delta bandwid stats to	*/
  char *dbw_fname;
  dbw_fname = strdup("dbw_stats0,0.dat");
  dbw_fname[9] = '0' + num_task;
  dbw_fname[11] = '0' + num_rs;
  dbw_time_stat.dumpStat(dbw_fname, "dbw", false, "Delta bandwidth");

  /** File where to dump sched err stats to	*/
  char *se_fname;
  se_fname = strdup("se_stats0,0.dat");
  se_fname[8] = '0' + num_task;
  se_fname[10] = '0' + num_rs;
  se_stat->dumpStat(se_fname, "se", ceil_model == true);

  /** File where to dump c_k stats to	*/
  char *ck_fname;
  ck_fname = strdup("ck_stats0,0.dat");
  ck_fname[8] = '0' + num_task;
  ck_fname[10] = '0' + num_rs;
  ck_stat->dumpStat(ck_fname, "ck", false, "Working time");

  /** File where to dump rsteps stats to	*/
  char *rsteps_fname;
  rsteps_fname = strdup("rs_stats0,0.dat");
  rsteps_fname[8] = '0' + num_task;
  rsteps_fname[10] = '0' + num_rs;
  rsteps_stat.dumpStat(rsteps_fname, "rs", false, "Return steps to negative");

  /** Prob of Invariant */
  char *pi_fname;
  pi_fname = strdup("pi_stats0,0.dat");
  pi_fname[8] = '0' + num_task;
  pi_fname[10] = '0' + num_rs;
  pinv_stat.dumpStat(pi_fname, "rs", false, "Prob{se in [-e,E]}");

  /** Prob of Invariant */
  char *pii_fname;
  pii_fname = strdup("pii_stats0,0.dat");
  pii_fname[9] = '0' + num_task;
  pii_fname[11] = '0' + num_rs;
  pinvi_stat.dumpStat(pii_fname, "rs", false, "Prob{se in [-ei,Ei]}");

  fprintf(stderr, "# pinv(%d,%d) = %g ([-e,E]=[%g,%g])\n",
  num_task, num_rs, pinv_stat.getMean(), -inv_e, inv_E);
  fprintf(stderr, "# pinvi(%d,%d) = %g ([-ei,Ei]=[%g,%g])\n",
  num_task, num_rs, pinvi_stat.getMean(), -inv_ei, inv_Ei);

  p_sched->dumpStats();
}

bool TaskScheduler::isRunning() {
  return (p_job_end != 0);
}

TaskScheduler::~TaskScheduler() {
  fclose(out_file);
}

void TaskScheduler::addPipelineNext(TaskScheduler *p_tsched) {
  pl_next.push_back(p_tsched);
}

void TaskScheduler::addPipelinePrev(TaskScheduler *p_tsched) {
  pl_prev.push_back(p_tsched);
}

bool TaskScheduler::isBlocked() {
  return pl_blocked;
}

bool TaskScheduler::checkPipelinePrevJobs(int next_job_id) {
  vector<TaskScheduler *>::iterator it = pl_prev.begin();
  for (; it != pl_prev.end(); it++) {
    Logger::debugLog("last_finished=%d, next=%d\n",
        (*it)->getLastFinishedJobID(), next_job_id);
    if ((*it)->getLastFinishedJobID() < next_job_id)
      return false;
  }
  return true;
}

void TaskScheduler::notifyPipelinePrevJobEnd(TaskScheduler *p_tsched) {
  if (!isBlocked())
    return;

  /* If all previous jobs finished, then start the waiting job in this task */
  if (checkPipelinePrevJobs(curr_job_id+1))
    addEventJobStart(0);
}

Time TaskScheduler::getStartTime() const {
  return start_time_offset;
}

bool TaskScheduler::checkParams() {
  Logger::debugLog("Checking task scheduler params\n");
  ASSERT(p_sched != 0, "No scheduler");
  ASSERT(getTask()->getMinExecutionTime() != UNASSIGNED, "Need to use -c");
  ASSERT(getTask()->getMaxExecutionTime() != UNASSIGNED, "Need to use -C");
  // TODO: these should be related with params from controller instance (PDNV)
  RPStatBased *p_tpred = dynamic_cast<RPStatBased *>(getController()->getTaskPredictor());
  if (p_tpred != 0) {
    double p = p_tpred->getPercentile();
    Logger::debugLog("Setting ck_perc_est.percentile to %g, as from RPStatBased task predictor", p);
    ck_perc_est_temp.setPercentile(p);
  } else
    ck_perc_est_temp.setPercentile(0.83);
  ck_perc_est_temp.setSampleSize(GlobalOptimizer::getInstance()->getOptPeriod() / getTask()->getPeriod());
  return p_sched->checkParams();
}

void TaskScheduler::clearHistory() {
  p_sched->clearHistory();
  ck_perc_est_temp.clearHistory();
}
