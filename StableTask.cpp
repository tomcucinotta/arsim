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

#include "StableTask.hpp"
#include "defaults.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>

StableTask::StableTask()
  : Task() {
  c_last = 0;
}

double StableTask::generateInstance() {
  double r = getRandom();
  if ((r >= 0.9) || (c_last == -0)) {
    r = getRandom();
    c_last = c_min + r * (c_max - c_min - 1) + 1;
  }
  return c_last;
}

void StableTask::usage() {
  // No special options for StableTask
  // Add probability of change as an option ?
}
