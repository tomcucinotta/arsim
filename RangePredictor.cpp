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

#include "RangePredictor.hpp"

#include "RPStaticRange.hpp"
#include "RPRangeTrace.hpp"
#include "RPStatBased.hpp"

RangePredictor * RangePredictor::getInstance(const char *s) {
  if (strcmp(s, "sr") == 0)
    return new RPStaticRange();
  else if (strcmp(s, "rt") == 0)
    return new RPRangeTrace();
  else if (strcmp(s, "sb") == 0)
    return new RPStatBased();
  return 0;
}

bool RangePredictor::parseArg(int& argc, char **& argv) {
  return false;
}

void RangePredictor::usage() {
  printf("    RANGE PREDICTOR OPTIONS\n");
  RPStatBased::usage();
  RPStaticRange::usage();
  RPRangeTrace::usage();
}
