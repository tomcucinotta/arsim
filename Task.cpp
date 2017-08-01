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

#include <ctime>
/* Interface includes */

#include "defaults.hpp"
#include "util.hpp"
#include "FileUtil.hpp"
#include "Task.hpp"

/* Implementation includes */

#include <stdlib.h>
#include <math.h>
#include <string.h>

int Task::next_task_id = 0;

Task::Task() {
  srandom(time(NULL));
  setParams(DEF_T, DEF_h, DEF_H);
  task_id = Task::next_task_id++;
  c_min = UNASSIGNED;
  c_max = UNASSIGNED;
  app_mode = 0;
  prev_app_mode = 0;
  app_mode_models.push_back(new LinearModel());
  p_app_mode_stats = new TimeStat(0.0, 2.0, 1.0);
}

Task::~Task() {
}

void Task::setParams(double T, double min, double max) {
  period = T;
  c_min = min;
  c_max = max;
}

double Task::generateInstance() {
  double r = getRandom();
  double c_ret = c_min + r * (c_max - c_min);;
  ASSERT((c_ret + DELTA >= c_min) && (c_ret - DELTA <= c_max), "Calculated instance execution time is outside limits");
  return c_ret;
}

double Task::generateWorkingTime() {
  double wt = generateInstance();
  return app_mode_models[app_mode]->estimate(wt);
}

void Task::usage() {
  printf("    TASK OPTIONS\n");
  printf("(-t any)   -T      Period (default %g)\n", DEF_T);
  printf("(-t any)   -c      Minimum c(k) value\n");
  printf("(-t any)   -C      Maximum c(k) value\n");
  printf("(-t any)   -t-md   q1,m1,q2,m2,... for additional app modes (w.r.t. 0.0/1.0)\n");
}

bool Task::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-T") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &period) == 1, "Expecting double as argument to -T option");
  } else if (strcmp(*argv, "-c") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_min) == 1, "Expecting double as argument to -c option");
  } else if (strcmp(*argv, "-C") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%lg", &c_max) == 1, "Expecting double as argument to -C option");
  } else if (strcmp(*argv, "-t-md") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    std::vector<double> coeffs;
    if (strncmp(*argv, "l,", 2) == 0) {
      parseList(*argv + 2, coeffs);
      CHECK(coeffs.size() > 0 && (coeffs.size() % 2 == 0),
	    "Expecting a positive, even number of coefficients as arg to -t-md option");
      std::vector<LinearModel*>::iterator it;
      for (it = app_mode_models.begin(); it != app_mode_models.end(); ++it)
	delete *it;
      app_mode_models.clear();
      // First mode corresponds always to identity model
      app_mode_models.push_back(new LinearModel());
      for (unsigned int i = 0; i < coeffs.size(); i += 2) {
	double q = coeffs[i];
	double m = coeffs[i + 1];
	Logger::debugLog("Adding app mode: q=%g,m=%g\n", q, m);
	app_mode_models.push_back(new LinearModel(q, m));
      }
    } else if (strncmp(*argv, "b,", 2) == 0) {
      parseList(*argv + 2, coeffs);
      CHECK(coeffs.size() > 0 && (coeffs.size() % 4 == 0),
	    "Expecting a positive 4-multiple number of coefficients as arg to -t-md option");
      std::vector<LinearModel*>::iterator it;
      for (it = app_mode_models.begin(); it != app_mode_models.end(); ++it)
	delete *it;
      app_mode_models.clear();
      // First mode corresponds always to identity model
      app_mode_models.push_back(new LinearModel());
      for (unsigned int i = 0; i < coeffs.size(); i += 2) {
	double q = coeffs[i];
	double m = coeffs[i + 1];
	Logger::debugLog("Adding app mode: q=%g,m=%g\n", q, m);
	app_mode_models.push_back(new LinearModel(q, m));
      }
    } else {
      CHECK(0, "Wrong model-type: first elem in coefficient list must be 'l' (Linear) or 'b' (BlockLinear)");
    }
//     delete p_app_mode_stats;
//     p_app_mode_stats = new TimeStat(0.0, double(coeffs.size() / 2), 1.0);
  } else
    return false;
  return true;
}

void Task::setAppMode(unsigned int mode) {
  prev_app_mode = app_mode;
  app_mode = mode;
//   p_app_mode_stats->addSample(mode);
}

LinearModel & Task::getModelForMode(unsigned int app_mode) {
  return *app_mode_models[app_mode];
}
