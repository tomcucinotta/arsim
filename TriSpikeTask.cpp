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

#include "TriSpikeTask.hpp"
#include "defaults.hpp"
#include "util.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define TRI_STEPS 64.0

TriSpikeTask::TriSpikeTask()
  : parent() {
  c_mean_last = double(c_min + c_max) / 2.0;
  //c_mean_last = c_max - 2.0*DEF_h_w*double(c_max - c_min);
  c_max_i = c_mean_last + DEF_tpe*(c_max - c_min);
  c_min_i = c_mean_last - DEF_tpe*(c_max - c_min);
  pr_full_range = DEF_P_FR;
  burst_countdown = 10;

  dc_abs = double(c_max-c_min)/double(TRI_STEPS);
  dc_last = dc_abs;
}

double TriSpikeTask::generateInstance() {
  /*
  if (burst_countdown > 0) {
    burst_countdown--;
    return c_max;
  }
  */
  c_mean_last += dc_last;
  if ( (c_mean_last >= c_max_i - 2.0*DEF_h_w*(c_max - c_min)) || (c_mean_last <= c_min_i + 2.0*DEF_h_w*(c_max - c_min)) ) {
    dc_last = -dc_last;
    c_mean_last += dc_last;
  }

  double r = getRandom();
  double min, max;
  if (r < pr_full_range) {
    min = c_min;
    max = c_max;
    burst_countdown=10;
  } else {
    min = getMinExecutionTimeI();
    max = getMaxExecutionTimeI();
  }
  r = getRandom();
  return min + r * (max - min);
}

void TriSpikeTask::usage() {
  printf("(-t tp)     -Ci     Internal c(k) pipe excursion maximum value\n");
  printf("(-t tp)     -ci     Internal c(k) pipe excursion minimum value\n");
}

bool TriSpikeTask::parseArg(int& argc, char **& argv) {
  // Override -c and -C parameters behaviour
  if (strcmp(*argv, "-c") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_min) == 1, "Expecting double as argument to -c option");
    c_mean_last = (c_min + c_max) / 2.0;
  } else if (strcmp(*argv, "-C") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_max) == 1, "Expecting double as argument to -C option");
    c_mean_last = (c_min + c_max) / 2.0;
  } else
    return parent::parseArg(argc, argv);
  return true;
}

double TriSpikeTask::getMinExecutionTimeI() const { Logger::debugLog("TriSpikeTask::getMinTimeI(): returning %g\n", c_mean_last - DEF_h_w*(c_max - c_min)); return (c_mean_last - DEF_h_w*(c_max - c_min)); }
double TriSpikeTask::getMaxExecutionTimeI() const { Logger::debugLog("TriSpikeTask::getMaxTimeI(): returning %g\n", c_mean_last + DEF_h_w*(c_max - c_min)); return c_mean_last + DEF_h_w*(c_max - c_min); }
double TriSpikeTask::getInfExecutionTimeIRatio() const { return c_min_i / (c_min_i + 2.0*DEF_h_w*(c_max - c_min)); }
