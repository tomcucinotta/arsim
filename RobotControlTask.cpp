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

#include "RobotControlTask.hpp"
#include "defaults.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

RobotControlTask::RobotControlTask()
  : Task() {
  /* Get your own first event at the start of simulation */

  Event *p_ev = makeEvent(0, this, &RobotControlTask::handleSimStart);
  EventList::events().insert(p_ev);
}

double RobotControlTask::generateInstance() {
  return 0;
}

/** Handle start of simulation. Remember to explicitly post whatever
 ** other event you may need, in case you have stuff that goes on
 ** asynchronously with the generateInstance() calls, e.g., robot
 ** movements updates.
 **/
void RobotControlTask::handleSimStart(const Event & ev) {
  /* Retrieve the current time */
  Time curr_time = EventList::getTime();

  Logger::debugLog("Hello world. Current time: %g", curr_time);
}

void RobotControlTask::usage() {
  // No special options for RobotControlTask (yet)
}

bool RobotControlTask::parseArg(int& argc, char **& argv) {
  return false;
}
