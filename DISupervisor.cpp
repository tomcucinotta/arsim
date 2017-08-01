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

#include "DISupervisor.hpp"
#include "DoubleInvariantController.hpp"
#include "TaskScheduler.hpp"
#include "ResourceManager.hpp"

#include <set>

/** Method for retrieving the currently required bandwidth, as well as the quantity defined at step (3) in paper */
pair<double, double> getBwMin(TaskScheduler *p_sched, set<TaskScheduler *> & S, set<TaskScheduler *> & T) {
  DoubleInvariantController * p_ctrl = dynamic_cast<DoubleInvariantController *>(p_sched->getController());
  if (S.find(p_sched) == S.end())
    return pair<double, double>(p_sched->getRequiredBandwidth(), p_ctrl->getBwRangeI().getMin());
  else if (T.find(p_sched) == S.end())
    return pair<double, double>(p_ctrl->getBwRangeI().getMin(), p_ctrl->getBwRange().getMin());
  else
    return pair<double, double>(p_ctrl->getBwRange().getMin(), 0.0);
}

void DISupervisor::updateStats(vector<TaskScheduler*>& tasks, set<TaskScheduler *> & S, set<TaskScheduler *> & T) {
  vector<TaskScheduler*>::iterator it;
  if (stat_bw_req_granted.size() == 0) {
    for (it = tasks.begin(); it != tasks.end(); ++it) {
      stat_bw_req_granted.push_back(new TimeStat(0.0, 2.0, 1.0, EventList::getTime()));
      stat_bw_req_scaled.push_back(new TimeStat(0.0, 2.0, 1.0, EventList::getTime()));
      stat_bw_req_scaled2.push_back(new TimeStat(0.0, 2.0, 1.0, EventList::getTime()));
    }
  }
  for (unsigned int i = 0; i < tasks.size(); ++i) {
    TaskScheduler *p_sched = tasks[i];
    if (S.find(p_sched) == S.end()) {
      stat_bw_req_granted[i]->addSample(1.0, EventList::getTime());
      stat_bw_req_scaled[i]->addSample(0.0, EventList::getTime());
      stat_bw_req_scaled2[i]->addSample(0.0, EventList::getTime());
    } else if (T.find(p_sched) == S.end()) {
      stat_bw_req_granted[i]->addSample(0.0, EventList::getTime());
      stat_bw_req_scaled[i]->addSample(1.0, EventList::getTime());
      stat_bw_req_scaled2[i]->addSample(0.0, EventList::getTime());
    } else {
      stat_bw_req_granted[i]->addSample(0.0, EventList::getTime());
      stat_bw_req_scaled[i]->addSample(0.0, EventList::getTime());
      stat_bw_req_scaled2[i]->addSample(1.0, EventList::getTime());
    }
  }
}

void DISupervisor::checkGlobalConstraint(vector<TaskScheduler*>& tasks) {
  double bw_sum = 0.0;
  vector<TaskScheduler*>::iterator it;
  for (it = tasks.begin(); it != tasks.end(); ++it)
    if ((*it)->isRunning())
      bw_sum += (*it)->getRequiredBandwidth();
  if (bw_sum <= getSpeed()) {
    /* Possibly assign the originally required bandwidth,
     * if a job end (or a job start with a new bandwidth less than
     * the previously requested one) caused an overload condition
     * to end.
     * 
     * This part corresponds to the step (4) in the paper.
     */
    for (it = tasks.begin(); it != tasks.end(); it++) {
      if ((*it)->isRunning())
        (*it)->changeCurrentBandwidth((*it)->getRequiredBandwidth());
    }
  } else {
    /* Apply compression algorithm  */
    Logger::debugLog("# Warning: bw_sum = %g: enforcing global constraint...\n", bw_sum);
    std::set<TaskScheduler *> S; //< Internal invariant not guaranteed, external unprecised
    std::set<TaskScheduler *> T; //< External invariant not guaranteed (only best effort apps)

    while (true) {

      Logger::debugLog("Set S: ");
      for (set<TaskScheduler *>::iterator sit = S.begin(); sit != S.end(); ++sit) {
	DoubleInvariantController * p_ctrl = dynamic_cast<DoubleInvariantController *>((*sit)->getController());
	Logger::debugLog("<%g,I:[%g,%g],[%g,%g]> ", (*sit)->getRequiredBandwidth(),
			 p_ctrl->getBwRangeI().getMin(), p_ctrl->getBwRangeI().getMax(),
			 p_ctrl->getBwRange().getMin(), p_ctrl->getBwRange().getMax());
      }
      Logger::debugLog("\n");

      Logger::debugLog("Set T: ");
      for (set<TaskScheduler *>::iterator sit = T.begin(); sit != T.end(); ++sit) {
	DoubleInvariantController * p_ctrl = dynamic_cast<DoubleInvariantController *>((*sit)->getController());
	Logger::debugLog("<%g,I:[%g,%g],[%g,%g]> ", (*sit)->getRequiredBandwidth(),
			 p_ctrl->getBwRangeI().getMin(), p_ctrl->getBwRangeI().getMax(),
			 p_ctrl->getBwRange().getMin(), p_ctrl->getBwRange().getMax());
      }
      Logger::debugLog("\n");

      double bw_min_sum = 0.0;
      bw_sum = 0.0;
      for (it = tasks.begin(); it != tasks.end(); ++it) {
        TaskScheduler *p_sched = (*it);
        DoubleInvariantController *p_ctrl = dynamic_cast<DoubleInvariantController *>(p_sched->getController());
        CHECK(p_ctrl != 0, "Not a double invariant controller with DISupervisor");
        if ((*it)->isRunning()) {
          pair<double, double> bws = getBwMin(*it, S, T);
          bw_sum += bws.first;
          bw_min_sum += bws.second;
        }
      }
      if (bw_min_sum <= getSpeed()) {
        /** This part corresponds to the step (5) in the paper.     */
        for (it = tasks.begin(); it != tasks.end(); ++it) {
          if ((*it)->isRunning()) {
            pair<double, double> bws = getBwMin(*it, S, T);
            double bw_req = bws.first;
            double bw_min = bws.second;
	    double bw = bw_min + (bw_req - bw_min) / (bw_sum - bw_min_sum) * (getSpeed() - bw_min_sum);
	    Logger::debugLog("Compressing bw to: %g (req=%g, min=%g)\n", bw, bw_req, bw_min);
            (*it)->changeCurrentBandwidth(bw);
          }
        }
        break;
      }

      /** Step (6): search for a task still with internal invariant guaranteed */
      bool found = false;
      for (it = tasks.begin(); it != tasks.end(); ++it) {
        DoubleInvariantController *p_ctrl = dynamic_cast<DoubleInvariantController *>((*it)->getController());
        if ((*it)->isRunning() && p_ctrl->isBestEffort() && S.find(*it) == S.end()) {
          S.insert(*it);
          found = true;
          break;
        }
      }
      if (found)
        continue;

      /** Step (7): search for a task from the guaranteed set and degrade the invariant (add it to the S set) */
      for (it = tasks.begin(); it != tasks.end(); ++it) {
        DoubleInvariantController *p_ctrl = dynamic_cast<DoubleInvariantController *>((*it)->getController());
        if ((*it)->isRunning() && (! p_ctrl->isBestEffort()) && S.find(*it) == S.end()) {
          S.insert(*it);
          found = true;
          break;
        }
      }
      if (found)
        continue;

      /** Step (8): search for a task from the best effort set and do not guarantee anything (add it to the T set) */
      for (it = tasks.begin(); it != tasks.end(); ++it) {
        DoubleInvariantController *p_ctrl = dynamic_cast<DoubleInvariantController *>((*it)->getController());
        if ((*it)->isRunning() && p_ctrl->isBestEffort() && T.find(*it) == T.end()) {
          T.insert(*it);
          found = true;
          break;
        }
      }
      if (found)
        continue;

      CHECK(false, "Invalid inputs: no way to ensure respect of promised guarantees");
    }
    updateStats(tasks, S, T);
  }
}

void DISupervisor::dumpStats() {
  Logger::debugLog("bw_req_granted: ");
  for (unsigned int i = 0; i < stat_bw_req_granted.size(); ++i)
    Logger::debugLog("%g ", stat_bw_req_granted[i]->getMean());
  Logger::debugLog("\n");

  Logger::debugLog("bw_req_scaled : ");
  for (unsigned int i = 0; i < stat_bw_req_scaled.size(); ++i)
    Logger::debugLog("%g ", stat_bw_req_scaled[i]->getMean());
  Logger::debugLog("\n");

  Logger::debugLog("bw_req_scaled2: ");
  for (unsigned int i = 0; i < stat_bw_req_scaled2.size(); ++i)
    Logger::debugLog("%g ", stat_bw_req_scaled2[i]->getMean());
  Logger::debugLog("\n");
}
