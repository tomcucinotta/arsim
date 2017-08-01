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

#include "TPValueTrace.hpp"
#include "util.hpp"
#include "FileUtil.hpp"

#include <stdio.h>
#include <string.h>
#include <malloc.h>

TPValueTrace::TPValueTrace() {
  trace_fname = 0;
  disc_lines = 0;
  col_number = 0;
  mul_factor = 1.0;
  curr_sample = samples.begin();
}

TPValueTrace::~TPValueTrace() {
  if (trace_fname != 0)
    free(trace_fname);
}

bool TPValueTrace::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-tvp-f") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    if (trace_fname != 0)
      free(trace_fname);
    trace_fname = strdup(*argv);
    ASSERT(trace_fname != NULL, "Out of memory");
    Logger::debugLog("ValueTrace fname: %s\n", trace_fname);
  } else if (strcmp(*argv, "-tvp-mul") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%lg", &mul_factor) == 1) && (mul_factor > 0.0), "Expecting positive real as argument to -tvp-mul option");
  } else if (strcmp(*argv, "-tvp-disc") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%d", &disc_lines) == 1) && (disc_lines >= 0), "Expecting positive integer as argument to -tvp-disc option");
  } else if (strcmp(*argv, "-tvp-c") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%d", &col_number) == 1) && (col_number >= 1), "Expecting integer >= 1 as argument to -tvp-c option");
  } else
    return false;
  return true;
}

void TPValueTrace::usage() {
  printf("(-vp tvp) -tvp-f   Value trace filename\n");
  printf("(-vp tvp) -tvp-c   Column to use from the trace file (1 is the first one)\n");
  printf("(-vp tvp) -tvp-mul Multiply factor (defaults to 1.0)\n");
  printf("(-vp tvp) -tvp-disc Lines to discard at head of trace file (defaults to 0)\n");
}

void TPValueTrace::calcParams() {
  parent::calcParams();
  if (samples.size() == 0) {
    loadTrace(samples, trace_fname, disc_lines, col_number, mul_factor);
    ASSERT(samples.size() > 0, "Could not load trace samples");
    curr_sample = samples.begin();
  }
}

/** Add a c_k sample.
 **
 ** @note
 ** Supplied sample completely ignored: prediction is taken from a
 ** trace. This method is used as a step method so that the next
 ** value prediction may be loaded from the trace file.
 **/
void TPValueTrace::addSample(double c_k) {
  ++curr_sample;
}

/** Return last read value from file trace.
 **
 ** @note
 ** This predictor returns a prediction from the very first invocation, so no -1 is returned,
 ** like it happens for other predictors that return -1 until they accumulate sufficient statistics.
 **/
double TPValueTrace::getExpValue() const {
  ASSERT(curr_sample != samples.end(), "Trace not loaded or premature EOT");
  return *curr_sample;
}
