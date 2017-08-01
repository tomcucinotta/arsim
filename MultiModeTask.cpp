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

/* Interface includes */

#include "TaskFactory.hpp"
#include "MultiModeTask.hpp"
#include "TraceTask.hpp"
#include "defaults.hpp"
#include "util.hpp"
#include "Events.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

void MultiModeTask::usage() {
  printf("(-t mmt)   -mmt-t  Sets new application-mode task type (refer to -t option args)\n");
  printf("(-t mmt)   -mmt-am Sets initial application-mode (0 is the first task defined with -mmt-t)\n");
}

MultiModeTask::MultiModeTask()
  : Task() {
  mmt_app_mode = 0;
  prev_mmt_app_mode = 0;
}

double MultiModeTask::generateWorkingTime() {
  double retval = 0.0;
  for (unsigned int am = 0; am < app_mode_tasks.size(); ++am) {
    Task *p_task = app_mode_tasks[am];
    double sample = p_task->generateWorkingTime();
    if (am == mmt_app_mode)
      retval = sample;
  }
  CHECK1(retval != 0.0, "Inconsistent mmt_app_mode: %u\n", mmt_app_mode);
  return retval;
}

bool MultiModeTask::parseArg(int& argc, char **& argv) {
  if ((app_mode_tasks.size() > 0) && (*(app_mode_tasks.end()-1))->parseArg(argc, argv)) {
    return true;
  } else if (strcmp(*argv, "-mmt-t") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    Task * p_new_task = TaskFactory::getInstance(*argv);
    CHECK(p_new_task != 0, "Unknown task type");
    Logger::debugLog("Adding subtask '%s' to multi-mode task %p\n", *argv, this);
    app_mode_tasks.push_back(p_new_task);
  } else
    return parent::parseArg(argc, argv);
  return true;
}

void MultiModeTask::setAppMode(unsigned int new_mode) {
  CHECK1(new_mode < app_mode_tasks.size(), "app mode %d out of range", new_mode);
  CHECK1(new_mode < app_mode_models.size(), "app mode %d out of range", new_mode);
  prev_mmt_app_mode = mmt_app_mode;
  mmt_app_mode = new_mode;
  p_app_mode_stats->addSample(new_mode, EventList::getTime());
}

bool MultiModeTask::checkParams() {
  if (! parent::checkParams())
    return false;
  for (unsigned int am = 0; am < app_mode_tasks.size(); ++am)
    BCHECK(app_mode_tasks[am]->checkParams(), "Consistency check on subtask failed");
  return true;
}

void MultiModeTask::calcParams() {
  parent::calcParams();
  for (unsigned int am = 0; am < app_mode_tasks.size(); ++am) {
    app_mode_tasks[am]->calcParams();
    double am_c_min = app_mode_tasks[am]->getMinExecutionTime();
    if (am_c_min != UNASSIGNED) {
      if (c_min == UNASSIGNED || am_c_min < c_min)
        c_min = am_c_min;
    }
    double am_c_max = app_mode_tasks[am]->getMaxExecutionTime();
    if (am_c_max != UNASSIGNED) {
      if (c_max == UNASSIGNED || am_c_max > c_max)
        c_max = am_c_max;
    }
    CHECK(period == app_mode_tasks[am]->getPeriod(), "Inconsistent period settings");
    /* <= is on-purpose */
    CHECK(am <= app_mode_models.size(), "Hmmm... something strange in calParams() invokation order");
    /* == is on-purpose */
    if (am == app_mode_models.size()) {
      LinearModel *p_mdl = new LinearModel();
      TraceTask const *p_ref_task = dynamic_cast<TraceTask *>(app_mode_tasks[0]);
      CHECK(p_ref_task != 0, "First mode is not a trace task ! Please, provide best-fit model by hand");
      TraceTask const *p_task = dynamic_cast<TraceTask *>(app_mode_tasks[am]);
      CHECK(p_task != 0, "Current mode is not a trace task ! Please, provide best-fit model by hand");
      vector<double>::const_iterator x_it_beg = p_ref_task->getSamples().begin();
      vector<double>::const_iterator x_it_end = p_ref_task->getSamples().end();
      vector<double>::const_iterator y_it_beg = p_task->getSamples().begin();
      vector<double>::const_iterator y_it_end = p_task->getSamples().end();
      p_mdl->fit(x_it_beg, x_it_end, y_it_beg, y_it_end);
      Logger::debugLog("Pushing auto-fitted model: m=%g, q=%g\n", p_mdl->getM(), p_mdl->getQ());
      app_mode_models.push_back(p_mdl);
    }
  }
  delete p_app_mode_stats;
  p_app_mode_stats = new TimeStat(0.0, app_mode_tasks.size(), 1.0, EventList::getTime());
  p_app_mode_stats->addSample(mmt_app_mode, EventList::getTime());
}
