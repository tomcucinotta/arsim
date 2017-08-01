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

#include "GlobalOptimizer.hpp"
#include "Events.hpp"
#include <stdio.h>
#include <string.h>
#include "util.hpp"
#include <sstream>
#include "ResourceManager.hpp"
#include <vector>
#include "qos_opt_glpk.h"
#include "qos_opt_heur.h"
#include "defaults.hpp"

GlobalOptimizer *GlobalOptimizer::p_gc = new GlobalOptimizer();

// Constructor
GlobalOptimizer::GlobalOptimizer()
 : obj_val_stat(0.0, 400.0, 1.0), perf_index_stat(0.0, 400.0, 1.0) {
  opt_type = "glpk";
  p_opt = NULL;
  na=0;
  nam=0;
  nr=0;
  nrm=0;
  use_mmp = 0;
  opt_period = 0;
  p_opt_ev = NULL;
  gc_file = NULL;
}

void GlobalOptimizer::dumpStatistics() {
  printf("# Opt Obj Val Min=%g, Avg=%g, Max=%g\n",
      obj_val_stat.getMin(), obj_val_stat.getMean(), obj_val_stat.getMax());
  printf("# Perf. Index Min=%g, Avg=%g, Max=%g\n",
      perf_index_stat.getMin(), perf_index_stat.getMean(), perf_index_stat.getMax());
  printf("# Modes statistics (per-mode PMF value):\n");

  printf("# ");
  for (int r = 0; r < nr; ++r) {
    for (int rm = 0; rm < nrm; ++rm) {
      printf("      r%drm%d", r, rm);
    }
    for (int a = 0; a < na; ++a) {
      for (int am = 0; am < nam; ++am) {
        printf("    r%da%dam%d", r, a, am);
      }
    }
  }
  printf("\n");

  printf("# ");
  vector<ResourceManager*>::iterator rs_it = ResourceManager::rs_controllers.begin();
  for (; rs_it != ResourceManager::rs_controllers.end(); ++rs_it) {
    for (int rm = 0; rm < nrm; ++rm) {
      printf("%11.5g", (*rs_it)->getPowerModeStatistics().getPMFValue(rm));
    }
    for (unsigned int app = 0; app < (*rs_it)->getTaskSchedulerNum(); ++app) {
      TaskScheduler *p_sched = (*rs_it)->getTaskSchedulerAt(app);
      for (int am = 0; am < nam; ++am) {
        printf("%11.5g", p_sched->getTask()->getAppModeStatistics().getPMFValue(am));
        fflush(stdout);
      }
    }
  }
  printf("\n");
}

GlobalOptimizer::~GlobalOptimizer() {
  if (p_opt_ev != NULL)
    EventList::events().remove(p_opt_ev);
  fclose(gc_file);
}

void GlobalOptimizer::usage() {
  printf("GLOBAL OPTIMIZER OPTIONS\n");
  printf("           -gc-type Provide global controller type (glpk/heur)\n");
  printf("           -gc-nums Provide comma-separated list of num_app,num_app_modes,num_res,num_res_modes\n");
  printf("           -gc-ql   app,app_mode,lev: associate a QoS level (0-100) to specified app and app_mode\n");
  printf("           -gc-qg   app,gain: associate a gain/weight to specified app\n");
  printf("           -gc-qvp  app,penalty: associate a QoS variation penalty for the specified app\n");
  printf("           -gc-am   app,app_mode: specify initial app_mode for app\n");
  printf("           -gc-pl   res,res_mode,lev: associate a QoS penalty (0-100) to specified res and res_mode\n");
  printf("           -gc-pp   res,weight: associate a penalty/weight to specified res\n");
  printf("           -gc-load a,am,r,rm: set initial load for the specified quadruple\n");
  printf("           -gc-p    period: set optimization period (and start periodic optimization)\n");
  printf("           -gc-max-pow power: set maximum allowed power (unimplemented yet)\n");
  printf("           -gc-it-lim num: set maximum number of iterations per optimization step\n");
  printf("           -mmp:    Activate Multi-Mode Predictor (disabled by default)\n");
}

bool GlobalOptimizer::parseArg(int& argc, char **& argv) {
  int app, app_mode, res, res_mode, lev;
  double load;
  long unsigned it_limit;
  if (strcmp(*argv, "-gc-type") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    opt_type = *argv;
  } else if (strcmp(*argv, "-gc-nums") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d,%d,%d,%d", &na, &nam, &nr, &nrm) == 4, "Expecting 4 comma-separated naturals as argument to -gc-nums option");
    Logger::debugLog("na=%d, nam=%d, nr=%d, nrm=%d\n", na, nam, nr, nrm);
    if (strcmp(opt_type, "glpk") == 0)
      p_opt = qos_opt_glpk_create(na, nam, nr, nrm);
    else if (strcmp(opt_type, "heur") == 0)
      p_opt = qos_opt_heur_create(na, nam, nr, nrm);
    else
      CHECK(0, "Invalid global controller type supplied to -gc-type option");

    CHECK(p_opt != NULL, "Invalid numbers supplied to -gc-nums or not enough memory");
    /* Setting default flat params */
    for (app = 0; app < na; ++app) {
      for (app_mode = 0; app_mode < nam; ++app_mode)
        // Default QoS Level for app_mode = 100 - app_mode
	qos_opt_set_qos_level(p_opt, app, app_mode, 100 - app_mode*10);
      // Default application weight = 1
      qos_opt_set_qos_gain(p_opt, app, 1);
      for (int am1 = 0; am1 < nam; ++am1)
        for (int am2 = 0; am2 < nam; ++am2)
          // Default QoS variation penalty = 0
          if (am1 != am2)
            qos_opt_set_qos_var_penalty(p_opt, app, am1, am2, 0);
      // Default application mode = 0 (first mode, maximum QoS)
      qos_opt_set_app_mode(p_opt, app, 0);
    }
    for (res = 0; res < nr; ++res) {
      for (res_mode = 0; res_mode < nrm; ++res_mode)
        // Default power level for res_mode = 100 - res_mode
	qos_opt_set_pow_level(p_opt, res, res_mode, 100 - res_mode*80);
      // Default weight for power consumption in objective function = 1
      qos_opt_set_pow_penalty(p_opt, res, 1);
    }
    for (app = 0; app < na; ++app)
      for (app_mode = 0; app_mode < nam; ++app_mode)
	for (res = 0; res < nr; ++res)
	  for (res_mode = 0; res_mode < nrm; ++res_mode)
	    // Default workload for each application and resource mode = 1/na
	    qos_opt_set_load(p_opt, app, app_mode, res, res_mode, 1.0 / na);
  } else if (strcmp(*argv, "-gc-ql") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d,%d,%d", &app, &app_mode, &lev) == 3, "Expecting 3 comma-separated integers as argument to -gc-ql option");
    qos_opt_set_qos_level(p_opt, app, app_mode, lev);
  } else if (strcmp(*argv, "-gc-qg") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d,%d", &app, &lev) == 2, "Expecting 2 comma-separated integers as argument to -gc-qg option");
    qos_opt_set_qos_gain(p_opt, app, lev);
  } else if (strcmp(*argv, "-gc-qvp") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d,%d", &app, &lev) == 2, "Expecting 2 comma-separated integers as argument to -gc-qvp option");
    for (int am1 = 0; am1 < nam; ++am1)
      for (int am2 = 0; am2 < nam; ++am2)
        if (am1 != am2)
          qos_opt_set_qos_var_penalty(p_opt, app, am1, am2, lev);
  } else if (strcmp(*argv, "-gc-am") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d,%d", &app, &app_mode) == 2, "Expecting 2 comma-separated integers as argument to -gc-am option");
    qos_opt_set_app_mode(p_opt, app, app_mode);
  } else if (strcmp(*argv, "-gc-pl") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d,%d,%d", &res, &res_mode, &lev) == 3, "Expecting 3 comma-separated integers as argument to -gc-pl option");
    qos_opt_set_pow_level(p_opt, res, res_mode, lev);
  } else if (strcmp(*argv, "-gc-pp") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d,%d", &res, &lev) == 2, "Expecting 2 comma-separated integers as argument to -gc-pp option");
    qos_opt_set_pow_penalty(p_opt, res, lev);
  } else if (strcmp(*argv, "-gc-load") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d,%d,%d,%d,%lg", &app, &app_mode, &res, &res_mode, &load) == 5, "Expecting 5 comma-separated numbers (4 int, 1 double) as argument to -gc-load option");
    qos_opt_set_load(p_opt, app, app_mode, res, res_mode, load);
  } else if (strcmp(*argv, "-gc-p") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lu", &opt_period) == 1, "Expecting integer as argument to -gc-p option");
    p_opt_ev = makeEvent((Time) opt_period, this, &GlobalOptimizer::handleUpdateBandEvent);
    EventList::events().insert(p_opt_ev);
  } else if (strcmp(*argv, "-gc-it-lim") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lu", &it_limit) == 1, "Expecting integer as argument to -gc-it-lim option");
    CHECK(qos_opt_set_it_limit(p_opt, it_limit) == 0, "Global optimizer does not support iteration limit");
  } else if (strcmp(*argv, "-gc-max-pow") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%d", &lev) == 1, "Expecting integer as argument to -gc-max-pow option");
    printf("UNSUPPORTED OPTION\n"); fflush(stdout);
    abort();
    //set_max_power(p_opt, lev);
  } else if (strcmp(*argv, "-mmp") == 0) {
    use_mmp = 1;
  } else
    return false;
  return true;
}

bool GlobalOptimizer::checkParams() {
  CHECK(p_opt != NULL, "Need to provide -gc-nums");
  Logger::debugLog("Solving optimization problem\n");
  optimize();
  Logger::debugLog("Solution (objective = %d):\n", qos_opt_get_obj_value(p_opt));
  for (int app = 0; app < na; ++app)
    Logger::debugLog("  app_mode[%d] = %d\n", app, qos_opt_get_app_mode(p_opt, app));
  for (int res = 0; res < nr; ++res)
    Logger::debugLog("  res_mode[%d] = %d\n", res, qos_opt_get_res_mode(p_opt, res));
  Logger::debugLog("Making Task application modes consistent with qos_opt\n");
  vector<ResourceManager*>::iterator rs_it = ResourceManager::rs_controllers.begin();
  for (; rs_it != ResourceManager::rs_controllers.end(); ++rs_it) {
    for (unsigned int app = 0; app < (*rs_it)->getTaskSchedulerNum(); ++app) {
      TaskScheduler *p_sched = (*rs_it)->getTaskSchedulerAt(app);
      int am = qos_opt_get_app_mode(p_opt, app);
      p_sched->getTask()->setAppMode(am);
    }
  }
  return true;
}

void GlobalOptimizer::optimize() {
  if (gc_file == NULL) {
    gc_file = fopen("gc.dat", "w");
    CHECK(gc_file != NULL, "Could not open file gc.dat");
    fprintf(gc_file, "# %9s", "Time");
    // Required bandwidths (observed loads)
    for (int r = 0; r < nr; ++r) // Resources
      for (int a = 0; a < na; ++a) // Applications
	for (int rm = 0; rm < nrm; ++rm) // Resource modes
	  for (int am = 0; am < nam; ++am) // Application modes
	    fprintf(gc_file, "%7s%d%d%d%d", "rbw", r, a, rm, am);
    // Application modes for each task on each resource
    for (int r = 0; r < nr; ++r) { // Resource
      fprintf(gc_file, "%10s%d", "rm", r);
      for (int a = 0; a < na; ++a) // Applications
        fprintf(gc_file, "%8s%d,%d", "am", r, a);
    }
    // Objective function value
    fprintf(gc_file, "%11s", "obj_val");
    // Experienced PDNV for each task on each resource
    for (int r = 0; r < nr; ++r) { // Resources
      for (int a = 0; a < na; ++a) // Applications
        fprintf(gc_file, "%8s%d,%d", "pi", r, a);
    }
    fprintf(gc_file, "\n");
  }
  fprintf(gc_file, "%11.5g", EventList::getTime());
  vector<ResourceManager*>::iterator rs_it = ResourceManager::rs_controllers.begin();
  for (; rs_it != ResourceManager::rs_controllers.end(); ++rs_it) {
    int res = rs_it - ResourceManager::rs_controllers.begin();
    for (unsigned int app = 0; app < (*rs_it)->getTaskSchedulerNum(); ++app) {
      TaskScheduler *p_sched = (*rs_it)->getTaskSchedulerAt(app);
      double period = p_sched->getTask()->getPeriod();
      int app_mode = p_sched->getTask()->getAppMode();
      int res_mode = (*rs_it)->getPowerMode();
      // Average Band required by the task
      // Obtained bandwidth is ABSOLUTE, i.e., already referred to resource at full speed of 1.0
      double band = p_sched->getRequiredBandwidthAvg();
      if (band < 0) {
	band = DEF_AVG_LOAD;
	Logger::debugLog("Warning: no bandwidth info from feedback -- assuming %g\n", band);
      }

      // Value for current modes is set independently: estimate(estimateInv()) is not always an identity
      Logger::debugLog("Setting new load for app %d mode %d res %d mode %d from %g to %g (measured)\n",
		       app, app_mode, res, res_mode, qos_opt_get_load(p_opt, app, app_mode, res, res_mode), band);
      qos_opt_set_load(p_opt, app, app_mode, res, res_mode, band);

      // Average working-time required by the task
      double avg_ck = band * period;
      // Update other app modes and res modes through respective models
      Task *task = p_sched->getTask();
      // Bandwidths in ARSim are already "absolute", i.e., expressed with respect to full resource speeds of 1.0
      // Example: speed is 0.5, I measured 0.3, thus I would occupy 30% at full speed.
      // Example: speed is 0.2, I measured 0.1, thus I would occupy 10% at full speed.
      double full_mode_avg_ck = task->getModelForMode(app_mode).estimateInv(avg_ck); // * (*rs_it)->getSpeedForMode(res_mode);
      Logger::debugLog("Measured a normalized band for app %d mode %d res %d mode %d of %g, (full app_mode=%g)\n",
          app, app_mode, res, res_mode, band, full_mode_avg_ck / period);
      for (int rm = 0; rm < nrm; ++rm) {
        for (int am = 0; am < nam; ++am) {
          // Here est_load is RELATIVE bandwidth (1.0 means full utilization at speed for rm)
          Logger::debugLog("Applying linear model: q=%g, m=%g\n", task->getModelForMode(am).getQ(), task->getModelForMode(am).getM());
          double est_avg_ck = task->getModelForMode(am).estimate(full_mode_avg_ck) / (*rs_it)->getSpeedForMode(rm);
          double est_load = est_avg_ck / period;
          if (int(p_sched->getTask()->getAppMode()) == am && res_mode == rm) {
            fprintf(gc_file, "%10.5f*", est_load);
            Logger::debugLog("band=%g, est_load=%g\n", band, est_load);
          } else
            fprintf(gc_file, "%11.5f", est_load);
	  if (use_mmp && am != app_mode && rm != res_mode) {
	    Logger::debugLog("Setting new load for app %d mode %d res %d mode %d from %g to %g\n",
			     app, am, res, rm, qos_opt_get_load(p_opt, app, am, res, rm), est_load);
	    qos_opt_set_load(p_opt, app, am, res, rm, est_load);
	  }
        }
      }
      Logger::debugLog("Clearing bw avg...\n");
      p_sched->clearRequiredBandwidthAvg();
    }
  }

  // Run the optimization engine
  Logger::debugLog("Optimizing...\n");
  bool solved = true;
  if (! qos_opt_solve(p_opt)) {
    solved = false;
    Logger::debugLog("...PROBLEM NOT SOLVED\n");
    // First (best) resource mode for every resource
    for (int r = 0; r < nr; ++r)
      qos_opt_set_res_mode(p_opt, r, 0);
    // Last (worst) application mode for every application
    for (int a = 0; a < na; ++a)
      qos_opt_set_app_mode(p_opt, a, nam - 1);
  }
  Logger::debugLog("...Done: objective function value: %g\n", qos_opt_get_obj_value(p_opt));

  for (int r = 0; r < nr; ++r) {
    double load_sum = 0.0;
    for (int a = 0; a < na; ++a)
      load_sum += qos_opt_get_load(p_opt, a, qos_opt_get_app_mode(p_opt, a), r, qos_opt_get_res_mode(p_opt, r));
    Logger::debugLog("GC: total load on resource %d: %g\n", r, load_sum);
  }

  rs_it = ResourceManager::rs_controllers.begin();
  for (; rs_it != ResourceManager::rs_controllers.end(); ++rs_it) {
    int res = rs_it - ResourceManager::rs_controllers.begin();
    int res_mode = qos_opt_get_res_mode(p_opt, res);
    ResourceManager *p = *rs_it;
    fprintf(gc_file, "%11d", res_mode);
    for (unsigned int app = 0; app < (*rs_it)->getTaskSchedulerNum(); ++app) {
      TaskScheduler *p_sched = p->getTaskSchedulerAt(app);
      Task *task = p_sched->getTask();
      unsigned int new_app_mode = qos_opt_get_app_mode(p_opt, app);
      // If optimizer worked fine, then these assignments are feasible:
      // @TODO CHECK HERE, PLEASE !!!!
      //band = p_sched->getRequiredAvgBandwidth();
      // Here est_load is RELATIVE bandwidth (1.0 means that would fully occupy resource under current speed)
      double est_load = qos_opt_get_load(p_opt, app, new_app_mode, res, res_mode);
      // Here est_bw is ABSOLUTE bandwidth
      double est_bw = est_load * p->getSpeedForMode(res_mode);
      if (solved) {
        Logger::debugLog("Setting app %d to app_mode %d (on res %d res_mode %d) and gua_bw %g (est_load was %g)\n",
            app, new_app_mode, res, res_mode, est_bw, est_load);
        p_sched->getController()->setMinBandwidth(est_bw);
      } else {
        /* Problem was unfeasible: reuse latest minimum bw assignments */
        Logger::debugLog("Setting app %d to app_mode %d (on res %d res_mode %d) but gua_bw untouched (est_load was %g)\n",
            app, new_app_mode, res, res_mode, est_load);
      }
      bool app_mode_changed = (task->getAppMode() != new_app_mode);
      // Invoke anycase, because updates prev_app_mode as well
      task->setAppMode(new_app_mode);
      if (app_mode_changed)
        p_sched->clearHistory();
      fprintf(gc_file, "%11d", new_app_mode);
    }
    Logger::debugLog("Setting res %d to power mode %d\n", res, res_mode);
    // At this time, I already changed the minimum bandwidths according to
    // latest required measurements, so the assert in setSpeed() cannot fail
    p->setPowerMode(res_mode);
  }
  double obj_val = 0.0;
  rs_it = ResourceManager::rs_controllers.begin();
  for (unsigned int app = 0; app < (*rs_it)->getTaskSchedulerNum(); ++app) {
    TaskScheduler *p_sched = (*rs_it)->getTaskSchedulerAt(app);
    int app_mode = p_sched->getTask()->getAppMode();
    int prev_app_mode = p_sched->getTask()->getPreviousAppMode();
    obj_val += qos_opt_get_qos_level(p_opt, app, app_mode) * qos_opt_get_qos_gain(p_opt, app);
    Logger::debugLog("obj_val QoS term: +%g (tot: %g)\n", qos_opt_get_qos_level(p_opt, app, app_mode) * qos_opt_get_qos_gain(p_opt, app), obj_val);
    if (prev_app_mode != app_mode) {
      obj_val -= qos_opt_get_qos_var_penalty(p_opt, app, prev_app_mode, app_mode);
      Logger::debugLog("obj_val QVP term: -%g (tot: %g)\n", qos_opt_get_qos_var_penalty(p_opt, app, prev_app_mode, app_mode), obj_val);
    }
    // fabs(get_qos_level(p_opt, app, app_mode) - get_qos_level(p_opt, app, prev_app_mode)) *
  }
  rs_it = ResourceManager::rs_controllers.begin();
  for (; rs_it != ResourceManager::rs_controllers.end(); ++rs_it) {
    int res = rs_it - ResourceManager::rs_controllers.begin();
    int res_mode = (*rs_it)->getPowerMode();
    obj_val -= (double) (int) qos_opt_get_pow_level(p_opt, res, res_mode); // * get_pow_penalty(p_opt, res);
    Logger::debugLog("obj_val PwP term: -%g (tot: %g)\n", (double) (int) qos_opt_get_pow_level(p_opt, res, res_mode), obj_val);
  }
  if (solved)
    Logger::debugLog("obj_val: %g (recomputed as %g, diff %g)\n", qos_opt_get_obj_value(p_opt), obj_val, qos_opt_get_obj_value(p_opt) - obj_val);
  obj_val_stat.addSample(obj_val);
  fprintf(gc_file, " %10g", obj_val);

  double pdnv_sum = 0.0;
  rs_it = ResourceManager::rs_controllers.begin();
  for (; rs_it != ResourceManager::rs_controllers.end(); ++rs_it) {
    for (unsigned int app = 0; app < (*rs_it)->getTaskSchedulerNum(); ++app) {
      TaskScheduler *p_sched = (*rs_it)->getTaskSchedulerAt(app);
      fprintf(gc_file, " %10g", p_sched->getTempPDNVStat().getMean());
      pdnv_sum += p_sched->getTempPDNVStat().getMean();
      p_sched->getTempPDNVStat().clear();
    }
  }
  double perf_index = obj_val * pdnv_sum / (na * nr);
  perf_index_stat.addSample(perf_index);
  Logger::debugLog("# perf_index: %g (avg: %g)\n", perf_index, perf_index_stat.getMean());
  fprintf(gc_file, "\n");
}

void GlobalOptimizer::handleUpdateBandEvent(const Event & ev) {
  optimize();
  Logger::debugLog("Scheduling next optimization period %d time units forward\n", opt_period);
  p_opt_ev = makeEvent(opt_period, this, &GlobalOptimizer::handleUpdateBandEvent);
  EventList::events().insert(p_opt_ev);
}
