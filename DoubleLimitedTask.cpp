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

#include "DoubleLimitedTask.hpp"
#include "defaults.hpp"
#include "util.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

DoubleLimitedTask::DoubleLimitedTask()
  : Task() {
  alpha_i = -1;
  calcIInterval();
}

void DoubleLimitedTask::calcIInterval() {
  c_max_i = (c_min + c_max)/2.0 + DEF_h_w*(c_max - c_min)/2.0;
  c_min_i = (c_min + c_max)/2.0 - DEF_h_w*(c_max - c_min)/2.0;
}

void DoubleLimitedTask::usage() {
  printf("(-t p|tr)  -Ci     Internal c(k) range maximum value\n");
  printf("(-t p|tr)  -ci     Internal c(k) range minimum value\n");
}

bool DoubleLimitedTask::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-ci") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_min_i) == 1, "Expecting double as argument to -ci option");
  } else if (strcmp(*argv, "-Ci") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_max_i) == 1, "Expecting double as argument to -Ci option");
  } else if (strcmp(*argv, "-ai") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &alpha_i) == 1, "Expecting double as argument to -ai option");
  } else if (strcmp(*argv, "-c") == 0) {
    /* override parent's behaviour */
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_min) == 1, "Expecting double as argument to -c option");
    calcIInterval();
  } else if (strcmp(*argv, "-C") == 0) {
    /* override parent's behaviour */
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_max) == 1, "Expecting double as argument to -C option");
    calcIInterval();
  } else
    return parent::parseArg(argc, argv);
  return true;
}

double DoubleLimitedTask::getInfExecutionTimeIRatio() const {
  if (alpha_i != -1)
    return alpha_i;
  else
    return c_min_i / c_max_i;
}
