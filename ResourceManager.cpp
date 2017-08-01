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

#include "ResourceManager.hpp"

#include "FileUtil.hpp"
#include "util.hpp"

#include <sstream>

vector<ResourceManager*> ResourceManager::rs_controllers;
int ResourceManager::next_rs_id = 0;

ResourceManager::ResourceManager() {
  Event *p_ev = makeEvent(0, this, &ResourceManager::handleSimStart);
  EventList::events().insert(p_ev);
  rs_controllers.push_back(this);
  rs_id = ResourceManager::next_rs_id++;
  std::ostringstream os;
  os << "CPU" << rs_id;
  rs_name = os.str();
  p_spv = NULL;
  Supervisor *p = Supervisor::getInstance("fair");
  CHECK(p != NULL, "No memory");
  setSupervisor(p);
  pow_mode = 0;
  p_pow_mode_stats = new TimeStat(0.0, 2.0, 1.0, EventList::getTime());
}

void ResourceManager::usage() {
  printf("  TASK MODEL AND CONTROLLER OPTIONS\n");
  printf("           -s      Set controller type: ls/la/ib/lak/re/fs/ey/di/pi/msse/oc\n");
  printf("           -sup    Set supervisor type: fair/di\n");
  printf("           -spd    Comma-separated list of additional resource power mode speeds (excluding the first one, 1.0)\n");
  printf("           -rm     Initial resource power-mode (defaults to 0)\n");
  TaskScheduler::usage();
  Supervisor::usage();
}

bool ResourceManager::parseArg(int& argc, char **& argv) {
  ASSERT(argc > 0, "Unexpected end of command-line options");

  if (strcmp(*argv, "-s") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    Controller *p_sched = Controller::getInstance(*argv);
    CHECK(p_sched != NULL, "Wrong scheduler type");

    p_sched->setResourceId(getResourceId());
    TaskScheduler *p_tsched = new TaskScheduler(p_sched, p_gsched);
    tasks.push_back(p_tsched);
    p_sched->setControllerId(getTaskSchedulerPos(p_tsched));
    Logger::debugLog("# Controller: %s\n", *argv);
  } else if (strcmp(*argv, "-sup") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    Supervisor *p_spv = Supervisor::getInstance(*argv);
    CHECK(p_spv != NULL, "Wrong supervisor type");
    setSupervisor(p_spv);
    Logger::debugLog("# Supervisor: %s\n", *argv);
  } else if (strcmp(*argv, "-spd") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    speeds.clear();
    speeds.push_back(1.0);
    parseList(*argv, speeds);
    Logger::debugLog("Read speeds: ");
    for (vector<double>::iterator it = speeds.begin(); it != speeds.end(); ++it)
      Logger::debugLog("%g, ", *it);
    Logger::debugLog("\n");
    delete p_pow_mode_stats;
    p_pow_mode_stats = new TimeStat(0.0, (double) speeds.size(), 1.0, EventList::getTime());
  } else if (strcmp(*argv, "-rm") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    unsigned int mode;
    CHECK(sscanf(*argv, "%u", &mode) == 1, "Expecting positive integer as argument to -rm option");
    CHECK(mode >= 0 && mode < speeds.size(), "Initial resource mode outside range of available speeds");
    setPowerMode(mode);
  } else if (p_spv->parseArg(argc, argv))
    ;
  else if (tasks.size() > 0) {
    TaskScheduler *p_tsched = tasks.back();
    return p_tsched->parseArg(argc, argv);
  } else {
    return false;
  }
  return true;
}

/** Destroy previously set supervisor, if any, and properly link new supervisor */
void ResourceManager::setSupervisor(Supervisor *p_spv) {
  CHECK(p_spv != NULL, "Trying to set a NULL supervisor");
  if (this->p_spv != NULL)
    delete this->p_spv;
  this->p_spv = p_spv;
  p_spv->setTasks(&this->tasks);
}

void ResourceManager::calcParamsAll() {
  vector<ResourceManager*>::iterator rs_it = rs_controllers.begin();
  for (; rs_it != rs_controllers.end(); ++rs_it)
    (*rs_it)->calcParams();
}

void ResourceManager::calcParams() {
  vector<TaskScheduler*>::iterator it = tasks.begin();
  for (; it != tasks.end(); ++it)
    (*it)->getController()->calcParams();
  p_pow_mode_stats->addSample(pow_mode, EventList::getTime());
}

bool ResourceManager::checkParamsAll() {
  vector<ResourceManager*>::iterator rs_it = rs_controllers.begin();
  for (; rs_it != rs_controllers.end(); ++rs_it) {
    if (! (*rs_it)->checkParams())
      return false;
  }
  return true;
}

bool ResourceManager::checkParams() {
  CHECK(speeds.size() > 0, "Need to use -spd option");
  vector<TaskScheduler*>::iterator it = tasks.begin();
  for (; it != tasks.end(); ++it)
    if (! (*it)->checkParams())
      return false;
  return true;
}

void ResourceManager::handleSimStart(const Event & ev) {
  vector<TaskScheduler*>::iterator it = tasks.begin();
  for (; it != tasks.end(); it++)
    (*it)->handleSimStart(ev);
}

// void ResourceManager::handleJobEnd(const Event & ev) {
//   Controller *p_sched = ev.p_scheduler;
//   double eps = p_sched->getError();
//   double eps_next = p_sched->step();
//   double delta_t = eps_next - eps;
//   Event *p_ev = makeEvent(delta_t, p_sched,
// 			  this, &ResourceManager::handleSimStart);
//   events.insert(p_ev);
// }

void ResourceManager::handleJobArrive(const Event & ev) {
  TaskScheduler *p_tsched = (TaskScheduler *) ev.p_data;
  ASSERT(p_tsched != 0, "No TaskScheduler defined for this event");
  p_tsched->handleJobArrive(ev);
  /* New job could have caused overload		*/
  p_spv->checkGlobalConstraint(tasks);
}

void ResourceManager::handleJobStart(const Event & ev) {
  TaskScheduler *p_tsched = (TaskScheduler *) ev.p_data;
  p_tsched->handleJobStart(ev);
  /* New job could have caused overload		*/
  p_spv->checkGlobalConstraint(tasks);
}

void ResourceManager::handleJobEnd(const Event & ev) {
  TaskScheduler *p_tsched = (TaskScheduler *) ev.p_data;
  p_tsched->handleJobEnd(ev);
  /* Ended job could cause overload end		*/
  p_spv->checkGlobalConstraint(tasks);
}

void ResourceManager::dump() {
  vector<ResourceManager*>::iterator rs_it = rs_controllers.begin();
  for (; rs_it != rs_controllers.end(); ++rs_it) {
    vector<TaskScheduler*>::iterator it = (*rs_it)->tasks.begin();
    for (; it != (*rs_it)->tasks.end(); ++it)
      (*it)->dump();
  }
}

void ResourceManager::dumpStatistics() {
  vector<ResourceManager*>::iterator rs_it = rs_controllers.begin();
  for (int r = 0; rs_it != rs_controllers.end(); ++rs_it, ++r) {
    vector<TaskScheduler*>::iterator it = (*rs_it)->tasks.begin();
    for (; it != (*rs_it)->tasks.end(); ++it)
      (*it)->dumpStatistics();

    /*  Dump model validation data	*/

    double mbw = 0.0;	// Average Max Bandwidth among tasks
    double rbw = 0.0;	// Average Required Bandwidth among tasks
    double cbw = 0.0;	// Average Current Bandwidth among tasks
    double dbw = 0.0;	// Average Delta Bandwidth among tasks
    double ase = 0.0;	// Average Scheduling Error among tasks
    it = (*rs_it)->tasks.begin();
    for (; it != (*rs_it)->tasks.end(); it++) {
      mbw += (*it)->getController()->getMaxBandwidth();
      rbw += (*it)->getMeanRequiredBandwidth();
      cbw += (*it)->getMeanBandwidth();
      dbw += (*it)->getMeanDeltaBandwidth();
      ase += (*it)->getMeanSchedError() / (*it)->getTask()->getPeriod();
    }
    mbw /= (*rs_it)->tasks.size();
    rbw /= (*rs_it)->tasks.size();
    cbw /= (*rs_it)->tasks.size();
    dbw /= (*rs_it)->tasks.size();
    ase /= (*rs_it)->tasks.size();

    ostringstream os;
    os << "validation" << r << ".dat";
    FILE *file = fopen(os.str().c_str(), "a");
    ASSERT(file != NULL, "Couldn't open file validation.dat for writing");
    fprintf(file, "%11.5f %11.5f %11.5f %11.5f %11.5f\n", mbw, rbw, cbw, dbw, ase);
    fclose(file);

    vector<ResourceManager*>::iterator gc_it = rs_controllers.begin();
    for (; gc_it != rs_controllers.end(); ++gc_it)
      (*gc_it)->p_spv->dumpStats();
  }
}

long ResourceManager::getLastFinishedJobID(unsigned int num_task) {
  CHECK(num_task < tasks.size(), "Need more tasks");
  return tasks[num_task]->getLastFinishedJobID();
}
ResourceManager::~ResourceManager() {
}

TaskScheduler *ResourceManager::getTaskSchedulerAt(unsigned int tsk) {
  if (tsk < tasks.size())
    return tasks[tsk];
  else
    return 0;
}

int ResourceManager::getTaskSchedulerPos(TaskScheduler *p_tsched) {
  vector<TaskScheduler *>::iterator it = tasks.begin();
  int tsk = 0;
  for (; it != tasks.end(); it++, tsk++)
    if (p_tsched == (*it))
      return tsk;
  return -1;
}

void ResourceManager::setResourceName(const char *rs_name) {
  this->rs_name = string(rs_name);
}

double ResourceManager::getSpeedForMode(int res_mode) const {
  CHECK1(res_mode < (int) speeds.size(), "Power mode %d out of range", res_mode);
  return speeds[res_mode];
}

void ResourceManager::setPowerMode(int mode) {
  Logger::debugLog("Trying to set new power mode %d with new speed: %g\n", mode, speeds[mode]);
  CHECK1(mode < (int) speeds.size(), "Power mode %d out of range", mode);
  pow_mode = mode;
  p_spv->setSpeed(speeds[mode]);
  p_pow_mode_stats->addSample(mode, EventList::getTime());
}

int ResourceManager::getPowerMode() const {
  return pow_mode;
}
