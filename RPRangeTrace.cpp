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

#include "RPRangeTrace.hpp"

#include <stdio.h>
#include <string.h>

RPRangeTrace::RPRangeTrace() {
  trace_fname = 0;
  ff = 0;
  num_samples = 0;
  curr_iv = Interval(0, 0);
}

RPRangeTrace::~RPRangeTrace() {
  if (trace_fname != 0)
    free(trace_fname);
}

bool RPRangeTrace::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-rt-tf") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    if (trace_fname != 0)
      free(trace_fname);
    trace_fname = strdup(*argv);
    ASSERT(trace_fname != NULL, "Out of memory");
    Logger::debugLog("RangeTrace fname: %s\n", trace_fname);
  } else
    return false;
  return true;
}

void RPRangeTrace::usage() {
  printf("(-tf tr)   -tf     Trace filename\n");
}

/* Add a c_k sample */
void RPRangeTrace::addSample(double c_k) {
  /* Supplied sample completely ignored: prediction is taken from a
   * trace. This method is used as a step method so that the next
   * interval may be loaded from the trace file
   */
  nextInterval();
}

/* Read next prediction interval from file */
void RPRangeTrace::nextInterval() {
  if (ff == 0) {
    printf("# Opening trace file: %s\n", trace_fname);
    ff = fopen(trace_fname, "r");
    ASSERT(ff != 0, "Couldn't find task trace file");
  }
  double lb, ub;
  char line_buf[256];
  if (fgets(line_buf, sizeof(line_buf), ff) != NULL) {
    while (strlen(line_buf) == 0
	   || sscanf(line_buf, "%lf %lf", &lb, &ub) != 2)
    {
      Logger::debugLog("Warning: skipping line: %s", line_buf);
      if (fgets(line_buf, sizeof(line_buf), ff) == NULL)
	break;
    }
  }
  if (lb < 0.0)
    lb = 0.0;
  if (ub < 0.0)
    ub = 0.0;
  curr_iv = Interval(lb, ub);
}

/* Return range in which next sample would reside with high probability */
Interval RPRangeTrace::getExpInterval() {
  if (ff == 0)	// Only occurrs for 1st sample
      nextInterval();

  return curr_iv;
}

///* Return middle point of prediction interval */
//double RPRangeTrace::getExpValue() {
//  return (curr_iv.getMin() + curr_iv.getMax()) / 2.0;
//}
//
///* Approximate standard deviation as width of prediction interval */
//double RPRangeTrace::getDevValue() {
//  return curr_iv.getMax() - curr_iv.getMin();
//}
//
///* Approximate positive deviation as half width of prediction interval */
//double RPRangeTrace::getDevPosValue() {
//  return (curr_iv.getMax() - curr_iv.getMin()) / 2.0;
//}
//
///* Approximate negative deviation as half width of prediction interval, negated */
//double RPRangeTrace::getDevNegValue() {
//  return - (curr_iv.getMax() - curr_iv.getMin()) / 2.0;
//}
