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
#include "TPStaticValue.hpp"

#include "util.hpp"
#include <math.h>

TPStaticValue::TPStaticValue() {
  c_mean = 0.0;
}

bool TPStaticValue::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-sv-mean") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_mean) == 1, "Expecting double as argument to -sv-mean option");
  } else
    return false;
  return true;
}

void TPStaticValue::usage() {
  printf("(-tp sp)   -sv-mean Mean value\n");
}

  /* Add a c_k sample */
void TPStaticValue::addSample(double c_k) {
  // Ignored on purpose
}

  /* Return expected next value */
double TPStaticValue::getExpValue() const {
  return c_mean;
}
