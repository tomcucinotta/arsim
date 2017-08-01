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
#include "Supervisor.hpp"

#include "FairSupervisor.hpp"
#include "DISupervisor.hpp"

Supervisor::Supervisor() {
  speed = 1.0;
  p_tasks = NULL;
}

void Supervisor::usage() {
  printf("    SUPERVISOR OPTIONS\n");
  printf("           -speed  Set initial resource speed\n");
  FairSupervisor::usage();
}

Supervisor * Supervisor::getInstance(const char * name) {
  if (strcmp(name, "fair") == 0)
    return new FairSupervisor();
  else if (strcmp(name, "di") == 0)
    return new DISupervisor();
  else
    return 0;
}

bool Supervisor::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-speed") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &speed) == 1, "Expecting double as argument to -speed option");
  } else {
    return false;
  }
  return true;
}

bool Supervisor::checkParams() {
  double bw_min_tot = 0.0;
  for (vector<TaskScheduler *>::iterator it = p_tasks->begin(); it != p_tasks->end(); ++it) {
    TaskScheduler *p_sched = (*it);
    bw_min_tot += p_sched->getController()->getMinBandwidth();
  }
  CHECK(bw_min_tot <= getSpeed(), "Total minimum guaranteed requests exceed resource speed");
  return true;
}
