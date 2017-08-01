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

#include "globals.hpp"

#include "ResourceManager.hpp"
#include "Controller.hpp"
#include "Task.hpp"
#include "TaskPredictor.hpp"
#include "util.hpp"
#include "defaults.hpp"
#include "GlobalOptimizer.hpp"

/* Implementation includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

ExitCond exit_cond = XC_JOB;
double x_time = 1000000;	/**< 1 second					*/
long int x_job = 10000;		/**< Exit at the end of the 10000th job...	*/
int x_tsk = 0;			/**< ...of the first defined task		*/
int x_rs = 0;			/**< ...within the first defined resource	*/
bool stat_only = false;         /**< Dump statistics only (do not dump time-by-time changes)     */

ResourceManager *p_gsched;	/**< Current ResourceManager while parsing args	*/

char const *prog_name;

void usage() {
  printf("\n");
  printf("Usage: %s [options]\n", prog_name);
  printf("  GENERAL OPTIONS\n");
  printf("           -h      Print this help message and exit\n");
  printf("           -xj j[,t[,r]] Exit at the specified job end\n");
  printf("           -xt     Exit at the specified time\n");
  printf("           -d      Enable log to specified file (defaults to /dev/null)\n");
  printf("           -r      Start definition of new resource\n");
  printf("           -rn     Set new resource name\n");
  printf("           -so     Dump statistics only (do not dump time-by-time changes)\n");
  ResourceManager::usage();
  GlobalOptimizer::usage();
  printf("\n");
}

bool parseArg(int & argc, char ** & argv) {
  if (strcmp(*argv, "-xj") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK(sscanf(*argv, "%ld,%d,%d", &x_job, &x_tsk, &x_rs) == 3
	  || sscanf(*argv, "%ld,%d", &x_job, &x_tsk) == 2
	  || sscanf(*argv, "%ld", &x_job) == 1,
	  "Wrong format for -xj option");
    exit_cond = XC_JOB;
    Logger::debugLog("# Setting x_job=%d,%d,%d\n", x_job,x_tsk,x_rs);
  } else if (strcmp(*argv, "-xt") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    CHECK((sscanf(*argv, "%lg", &x_time) == 1) && (x_time > 0), "Expecting positive real as argument to -xt option");
    exit_cond = XC_TIME;
    Logger::debugLog("# Setting x_time=%g\n", x_time);
  } else if (strcmp(*argv, "-d") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    fprintf(stderr, "# Enabling debug log to file '%s'\n", *argv);
    Logger::setLogFile(*argv);
  } else if (strcmp(*argv, "-r") == 0) {
    p_gsched = new ResourceManager();
  } else if (strcmp(*argv, "-rn") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    p_gsched->setResourceName(*argv);
  } else if (strcmp(*argv, "-so") == 0) {
    stat_only = true;
  } else if ((strcmp(*argv, "-h") == 0) || (strcmp(*argv, "--help") == 0)) {
    usage();
    exit(-1);
  } else if (p_gsched->parseArg(argc, argv)) {
    ;
  } else if (GlobalOptimizer::getInstance()->parseArg(argc, argv)) {
    ;
  } else
    return false;
  return true;
}

int main(int argc, char ** argv) {
  prog_name = argv[0];
  p_gsched = new ResourceManager();

  //Logger::setLogFile("/dev/null");
  //Logger::setLogFile("log.txt.gz");

  argv++;  argc--;
  while (argc > 0) {
    if (parseArg(argc, argv)) {
      argv++;  argc--;
    } else {
      printf("Unknown option: %s\n", *argv);
      exit(-1);
    }
  }

  p_gsched = 0;		// Global should not be used anymore from now on

  ResourceManager::calcParamsAll();
  GlobalOptimizer::getInstance()->calcParams();
  CHECK(ResourceManager::checkParamsAll(), "Scheduling not possible with provided and default parameters");
  CHECK(GlobalOptimizer::getInstance()->checkParams(), "Simulation impossible with current GlobalOptimizer parameters");

  bool eos = false;		// End Of Simulation
  double last_progress = 0.0;	// Progress at the last user-notified value
  double end_progress = 0.0;	// Progress at the end of simulation
  do {
    EventList::events().step();
    if (! stat_only)
      ResourceManager::dump();
    double curr_progress = 0.0;	// Current value of the simulation progress
    switch (exit_cond) {
    case XC_TIME:
      curr_progress = EventList::getTime();
      end_progress = x_time;
      break;
    case XC_JOB:
      curr_progress = ResourceManager::rs_controllers[0]->getLastFinishedJobID(0);
      end_progress = x_job;
      break;
    }
    double delta = end_progress / 16;
	//fprintf(stderr, "%g/%g\n", curr_progress, end_progress);
    while (curr_progress - last_progress > delta) {
      fprintf(stderr, ".");
      fflush(stderr);
      last_progress += delta;
    }
    if (curr_progress >= end_progress)
      eos = true;
  } while (! eos);

//   int st = N >> 4;
//   for (int k = 0; k < N; k++) {
//     p_gsched->step();
//     if (k % st == 0) {
//       fprintf(stderr, ".");
//       fflush(stderr);
//     }
//   }
  fprintf(stderr, "\n");
  ResourceManager::dumpStatistics();
  GlobalOptimizer::getInstance()->dumpStatistics();

  Logger::close();

  Logger::debugLog("Exiting with normal exit status");
  return 0;
}
