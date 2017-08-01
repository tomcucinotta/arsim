/* Implementation includes */

#include "GlobalOptimizer.hpp"
#include "util.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool parseArg(int & argc, char ** & argv) {
  if (GlobalOptimizer::getInstance()->parseArg(argc, argv)) {
    ;
  } else
    return false;
  return true;
}

int main(int argc, char ** argv) {
  Logger::setLogFile("log.txt");

  argv++;  argc--;
  while (argc > 0) {
    if (GlobalOptimizer::getInstance()->parseArg(argc, argv)) {
      argv++;  argc--;
    } else {
      printf("Unknown option: %s\n", *argv);
      exit(-1);
    }
  }

  CHECK(GlobalOptimizer::getInstance()->checkParams(), "Check failed");

  return 0;
}
