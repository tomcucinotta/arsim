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

#include <cstring>
#include "RPStaticRange.hpp"

#include <math.h>

RPStaticRange::RPStaticRange() {
  c_min = c_max = c_min_i = c_max_i = 0.0;
}

bool RPStaticRange::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-sr-min") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_min) == 1, "Expecting double as argument to -sr-min option");
  } else if (strcmp(*argv, "-sr-max") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_max) == 1, "Expecting double as argument to -sr-max option");
  } else if (strcmp(*argv, "-sr-min-i") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_min_i) == 1, "Expecting double as argument to -sr-min-i option");
  } else if (strcmp(*argv, "-sr-max-i") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_max_i) == 1, "Expecting double as argument to -sr-max-i option");
  } else
    return false;
  return true;
}

void RPStaticRange::usage() {
  printf("(-rp sr)   -sr-min  Absolute minimum\n");
  printf("(-rp sr)   -sr-max  Absolute maximum\n");
  printf("(-rp sr)   -sr-min-i  High probability lower bound\n");
  printf("(-rp sr)   -sr-max-i  High probability upper bound\n");
}

  /* Add a c_k sample */
void RPStaticRange::addSample(double c_k) {
  // Ignored on purpose
}

  /* Return range in which next sample would reside with high probability */
Interval RPStaticRange::getExpInterval() {
  return Interval(c_min, c_max);
}

  /* Return range in which next sample would reside with high probability */
Interval RPStaticRange::getExpIntervalI() {
  return Interval(c_min_i, c_max_i);
}
