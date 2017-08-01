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

#include "SpikeTask.hpp"
#include "defaults.hpp"
#include "util.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

SpikeTask::SpikeTask()
  : DoubleLimitedTask() {
  pr_full_range = DEF_P_FR;
}

/* Now pr_full_range is really probability of outside internal range */
double SpikeTask::generateInstance() {
  double r = getRandom();
  double min, max;
  double ret;
  if (r < pr_full_range) {
    max = c_max - c_max_i + c_min_i - c_min;
    r = getRandom() * max;
    if (r < c_min_i - c_min) {
      ret = c_min + r;
    } else {
      ret = c_max_i + (r - (c_min_i - c_min));
    }
  } else {
    min = c_min_i;
    max = c_max_i;
    r = getRandom();
    ret = min + r * (max - min);
  }
  return ret;
}

void SpikeTask::usage() {
  printf("(-t p)     -pf     Probability of spike\n");
}

bool SpikeTask::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-pf") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &pr_full_range) == 1, "Expecting double as argument to -pf option");
  } else
    return parent::parseArg(argc, argv);
  return true;
}
