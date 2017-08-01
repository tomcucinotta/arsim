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

#include "CmdlineTask.hpp"
#include "FileUtil.hpp"
#include "defaults.hpp"
#include "util.hpp"
#include "globals.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <values.h>
#include <algorithm>

CmdlineTask::CmdlineTask()
  : Task() {
  c_min = MAXDOUBLE;
  c_max = MINDOUBLE;
}

void CmdlineTask::usage() {
  printf("(-t cl)    -cl-s   Add manually comma-separated execution time samples\n");
}

bool CmdlineTask::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-cl-s") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    parseList(*argv, samples);
    c_min = *(std::min_element(samples.begin(), samples.end()));
    c_max = *(std::max_element(samples.begin(), samples.end()));
    it = samples.begin();
    extern long int x_job;
    extern ExitCond exit_cond;
    x_job = samples.size();
    exit_cond = XC_JOB;
  } else
    return parent::parseArg(argc, argv);
  return true;
}

double CmdlineTask::generateInstance() {
  double sample = *it++;
  if (it == samples.end())
    it = samples.begin();
  Logger::debugLog("Returning: %g\n", sample);
  return sample;
}
