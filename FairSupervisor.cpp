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
#include "FairSupervisor.hpp"

FairSupervisor::FairSupervisor() {
  soft = false;
}

void FairSupervisor::usage() {
  printf("(-sup fair) -soft  Enable soft reservations\n");
}

bool FairSupervisor::parseArg(int& argc, char **& argv) {
  if (parent::parseArg(argc, argv))
    return true;
  if (strcmp(*argv, "-soft") == 0) {
    soft = true;
  } else {
    return false;
  }
  return true;
}

void FairSupervisor::checkGlobalConstraint(vector<TaskScheduler*>& tasks) {
  double bw_req_sum = 0.0;	// Sum of required values
  double bw_req_wsum = 0.0;	// Sum of weighted required values
  double bw_min_sum = 0.0;      // Sum of configured minimum guaranteed values
  double bw_gua_sum = 0.0;	// Sum of actual guaranteed values
  double bw_gua_wsum = 0.0;	// Sum of weighted guaranteed values
  double weight_sum = 0.0;	// Sum of weights
  vector<TaskScheduler*>::iterator it = tasks.begin();
  for (; it != tasks.end(); ++it) {
    double bw_req;
    if ((*it)->isRunning())
      bw_req = (*it)->getRequiredBandwidth();
    else
      bw_req = 0.0;
    bw_req_sum += bw_req;

    double bw_min = (*it)->getController()->getMinBandwidth();
    bw_min_sum += bw_min;
    double bw_gua = std::min(bw_req, bw_min);
    bw_gua_sum += bw_gua;
    double weight = (*it)->getWeight();
    weight_sum += weight;
    bw_req_wsum += weight * bw_req;
    bw_gua_wsum += weight * bw_gua;
  }
  Logger::debugLog("bw_req_sum=%g, bw_gua_sum=%g, bw_min_sum=%g, speed=%g\n", bw_req_sum, bw_gua_sum, bw_min_sum, getSpeed());
  if (bw_req_sum > getSpeed()) {
    /* Apply compression algorithm	*/
    Logger::debugLog("# Warning: bw_sum = %g: enforcing global constraint...\n", bw_req_sum);
    // Use some tolerance in this assertion check
    ASSERT(bw_req_sum + 0.0001 >= bw_gua_sum, "Sum of requests below minimum guarantees but resource speed is not enough !");
    // Total available bandwidth after assigning minimum guarantees
    double bw_avail = getSpeed() - bw_gua_sum;
    // Use some tolerance in this assertion check
    ASSERT(bw_avail + 0.0001 >= 0, "Sum of minimum guarantees exceeding resource speed !");
    for (it = tasks.begin(); it != tasks.end(); ++it)
      if ((*it)->isRunning()) {
        TaskScheduler * p_sched = *it;
        double bw_req = p_sched->getRequiredBandwidth();
        double bw_min = p_sched->getController()->getMinBandwidth();
        double bw_gua = std::min(bw_req, bw_min);
	double weight = p_sched->getWeight();
        p_sched->changeCurrentBandwidth(
            bw_gua + bw_avail * weight * (bw_req - bw_gua) / (bw_req_wsum - bw_gua_wsum)
        );
      }
  } else {
    if (! soft) {
      /* Possibly assign the originally required bandwidth,
       * if a job end (or a job start with a new bandwidth less than
       * the previously requested one) caused an overload condition
       * to end
       */
      for (it = tasks.begin(); it != tasks.end(); ++it)
	if ((*it)->isRunning())
	  (*it)->changeCurrentBandwidth((*it)->getRequiredBandwidth());
    } else {
      double bw_avail = getSpeed() - bw_req_sum;
      for (it = tasks.begin(); it != tasks.end(); ++it)
	if ((*it)->isRunning()) {
	  TaskScheduler * p_sched = *it;
	  double bw_req = p_sched->getRequiredBandwidth();
	  double weight = p_sched->getWeight();
	  p_sched->changeCurrentBandwidth(
		bw_req + bw_avail * weight / weight_sum
	  );
	}
    }
  }
}
