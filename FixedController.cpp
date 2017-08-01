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

#include "FixedController.hpp"
#include "Interval.hpp"
#include "util.hpp"
#include "defaults.hpp"
#include "TriSpikeTask.hpp"

/* Implementation related includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

FixedController::FixedController()
  : parent() {
}

bool FixedController::checkParams() {
  if (! parent::checkParams())
    return false;
  return true;
}

/* Calculate scheduler dependent parameters */
void FixedController::calcParams() {
  parent::calcParams();
}

void FixedController::usage() {
}

bool FixedController::parseArg(int& argc, char **& argv) {
  return parent::parseArg(argc, argv);
}

double FixedController::calcBandwidth(double sched_err, double start_err) {
  return bw_max;
}
