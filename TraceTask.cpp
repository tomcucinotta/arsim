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

#include "TraceTask.hpp"
#include "defaults.hpp"
#include "util.hpp"
#include "FileUtil.hpp"
#include "Stat.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <numeric>
#include <sstream>

using namespace std;

TraceTask::TraceTask()
  : DoubleLimitedTask() {
  trace_fname = strdup(DEF_TFNAME);
  ASSERT(trace_fname != 0, "Out of memory");
  mul_factor = 1.0;
  disc_lines = 0;
  curr_sample = samples.begin();
  col_number = 0;
  sat_p_min = 0.0;
  sat_p_max = 1.0;
}

TraceTask::~TraceTask() {
  if (trace_fname != 0)
    free(trace_fname);
}

void TraceTask::loadTrace() {
  ASSERT1(::loadTrace(samples, trace_fname, disc_lines, col_number, mul_factor) > 0, "Could not load trace file %s", trace_fname);

  if (sat_p_min > 0.0 || sat_p_max < 1.0)
    fprintf(stderr, "# Saturated %lu samples\n", Stat::saturateAtPercentiles(samples.begin(), samples.end(), sat_p_min, sat_p_max));

  if (c_min == UNASSIGNED) {
    vector<double>::iterator it = min_element(samples.begin(), samples.end());
    c_min = *it;
    Logger::debugLog("min(c_k)=%g\n", c_min);
  }
  if (c_max == UNASSIGNED) {
    vector<double>::iterator it = max_element(samples.begin(), samples.end());
    c_max = *it;
    Logger::debugLog("max(c_k)=%g\n", c_max);
  }
  double avg = accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
  fprintf(stderr, "# Trace file (scaled by %g): min= %g max= %g avg= %g\n", mul_factor, c_min/period, c_max/period, avg/period);
  fprintf(stderr, "# Samples: Percentiles (00, 05, 10, 15, 85, 90, 95, 99.95, 100) = ( %g %g %g %g %g %g %g %g %g )\n",
	  (*min_element(samples.begin(), samples.end()))/period,
	  Stat::getMinPercentile(samples, 0.95, c_min, c_max)/period,
	  Stat::getMinPercentile(samples, 0.90, c_min, c_max)/period,
	  Stat::getMinPercentile(samples, 0.85, c_min, c_max)/period,
	  Stat::getMaxPercentile(samples, 0.85, c_min, c_max)/period,
	  Stat::getMaxPercentile(samples, 0.90, c_min, c_max)/period,
	  Stat::getMaxPercentile(samples, 0.95, c_min, c_max)/period,
	  Stat::getMaxPercentile(samples, 0.9995, c_min, c_max)/period,
	  (*max_element(samples.begin(), samples.end()))/period);
  // Output statistics on moving avg on 3 samples
  vector<double> mm;
  int k = 3;
  ASSERT(int(samples.size()) >= k, "Too short trace");
  for (int i = 0; i < (int)samples.size() - k + 1; i++)
    mm.push_back(accumulate(samples.begin() + i, samples.begin() + i + k, 0.0) / k);
  double mm_min = *min_element(mm.begin(), mm.end());
  double mm_max = *max_element(mm.begin(), mm.end());
  double mm_avg = accumulate(mm.begin(), mm.end(), 0.0) / mm.size();
  fprintf(stderr, "# Moving Avg on %d samples (scaled by %g): min= %g max= %g avg= %g\n", k, mul_factor, mm_min/period, mm_max/period, mm_avg/period);
  fprintf(stderr, "# Moving Avg (%d): Percentiles (00, 05, 10, 15, 85, 90, 95, 99.95, 100) = ( %g %g %g %g %g %g %g %g %g )\n", k,
	  (*min_element(mm.begin(), mm.end()))/period,
	  Stat::getMinPercentile(mm, 0.95, mm_min, mm_max)/period,
	  Stat::getMinPercentile(mm, 0.90, mm_min, mm_max)/period,
	  Stat::getMinPercentile(mm, 0.85, mm_min, mm_max)/period,
	  Stat::getMaxPercentile(mm, 0.85, mm_min, mm_max)/period,
	  Stat::getMaxPercentile(mm, 0.90, mm_min, mm_max)/period,
	  Stat::getMaxPercentile(mm, 0.95, mm_min, mm_max)/period,
	  Stat::getMaxPercentile(mm, 0.9995, mm_min, mm_max)/period,
	  (*max_element(mm.begin(), mm.end()))/period);
}

double TraceTask::generateInstance() {
  double sample;
  if (curr_sample == samples.end()) {
    fprintf(stderr, "Reached EOF of %s\n", trace_fname);
    return 0.0;
  }

  sample = *curr_sample;
  ++curr_sample;
  return sample;
}

void TraceTask::usage() {
  printf("(-t tr)    -tf     Trace filename\n");
  printf("(-t tr)    -tc     Column to use from the trace file (1 is the first one)\n");
  printf("(-t tr)    -mul    Multiply factor (defaults to 1.0)\n");
  printf("(-t tr)    -disc   Lines to discard at head of trace file (defaults to 0)\n");
  printf("(-t tr)    -tr-s p Saturate input at specified top distribution percentile (defaults to 1.0)\n");
  printf("(-t tr)    -tr-S p Saturate input at specified distribution percentile (defaults to 0.0)\n");
}

bool TraceTask::parseArg(int& argc, char **& argv) {
  if (parent::parseArg(argc, argv))
    return true;
  if (strcmp(*argv, "-tf") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    free(trace_fname);
    trace_fname = strdup(*argv);
    CHECK(sscanf(*argv, "%s", trace_fname) == 1, "Expecting filename as argument to -tf option");
    Logger::debugLog("Trace fname: %s\n", trace_fname);
  } else if (strcmp(*argv, "-mul") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%lg", &mul_factor) == 1) && (mul_factor > 0.0), "Expecting positive real as argument to -mul option");
  } else if (strcmp(*argv, "-disc") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%d", &disc_lines) == 1) && (disc_lines >= 0), "Expecting positive integer as argument to -disc option");
  } else if (strcmp(*argv, "-tc") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%d", &col_number) == 1) && (col_number >= 1), "Expecting integer >= 1 as argument to -tc option");
  } else if (strcmp(*argv, "-tr-s") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%lg", &sat_p_min) == 1) && (sat_p_min >= 0.0 && sat_p_min <= 1.0), "Expecting positive real in the [0,1] range as argument to -tr-s option");
  } else if (strcmp(*argv, "-tr-S") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%lg", &sat_p_max) == 1) && (sat_p_max >= 0.0 && sat_p_max <= 1.0), "Expecting positive real in the [0,1] range as argument to -tr-S option");
  } else
    return false;
  return true;
}

bool TraceTask::checkParams() {
  if (! parent::checkParams())
    return false;
  BCHECK(c_min <= c_max, "Bad min/max values");
  BCHECK(sat_p_min <= sat_p_max, "Bad saturation values");
  return true;
}

void TraceTask::calcParams() {
  parent::calcParams();
  if (samples.size() == 0) {
    loadTrace();
    ASSERT(samples.size() > 0, "Could not load trace samples");
    curr_sample = samples.begin();
  }
}
