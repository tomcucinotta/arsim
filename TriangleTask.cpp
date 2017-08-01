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

#include "TriangleTask.hpp"
#include "defaults.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

void TriangleTask::usage() {
  // No special options for triangular task
  // Add 'dc' as option ?
}

TriangleTask::TriangleTask()
  : Task() {
  c_last = (DEF_h+DEF_H)/2.0;
  dc_last = (DEF_H-DEF_h)/10.0;
}

double TriangleTask::generateInstance() {
  c_last += dc_last;
  if ((c_last > c_max) || (c_last < c_min)) {
    dc_last = -dc_last;
    c_last += dc_last;
  }
  return c_last;
}

void TriangleTask::setParams(double min_c, double max_c) {
  c_last = 0;
  dc_last = (c_max - c_min)/10;
}
