#include <Controller.hpp>
#include <SDBController.hpp>

#include <string>
#include <string.h>
#include <assert.h>

int main(int argc, char *argv[]) {
  Controller *p_ctrl = new SDBController();
  argc--;  argv++;
  while (argc > 0) {
    if (p_ctrl->parseArg(argc, argv)) {
      argv++;  argc--;
    } else {
      printf("Unknown option: %s\n", *argv);
      exit(-1);
    }
  }
  printf("Arguments parsed\n");

  p_ctrl->setLastJobExecTime(10.0);
  printf("bw=%g\n", p_ctrl->calcBandwidth(0.0, 0.0));

  p_ctrl->setLastJobExecTime(10.0);
  printf("bw=%g\n", p_ctrl->calcBandwidth(0.0, 0.0));

  p_ctrl->setLastJobExecTime(10.0);
  printf("bw=%g\n", p_ctrl->calcBandwidth(0.0, 0.0));

  return 0;
}
