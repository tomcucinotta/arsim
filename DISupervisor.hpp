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

#ifndef DISUPERVISOR_HPP_
#define DISUPERVISOR_HPP_

#include "Supervisor.hpp"
#include <set>

class DISupervisor : public Supervisor {

  vector<TimeStat *> stat_bw_req_granted;
  vector<TimeStat *> stat_bw_req_scaled;
  vector<TimeStat *> stat_bw_req_scaled2;
  void updateStats(vector<TaskScheduler*>& tasks, set<TaskScheduler *> & S, set<TaskScheduler *> & T);

public:

  void checkGlobalConstraint(vector<TaskScheduler*>& tasks);
  virtual void dumpStats();
};

#endif /*DISUPERVISOR_HPP_*/
