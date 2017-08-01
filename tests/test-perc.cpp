#include <Controller.hpp>
#include <PDNVController.hpp>

#include <string>
#include <string.h>
#include <assert.h>

int main(int argc, char *argv[]) {
  Controller *p_ctrl = new PDNVController();
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

  for (double c = 20.0; c < 30; c += 1.0) {
    p_ctrl->setLastJobExecTime(c);
    printf("bw=%g\n", p_ctrl->calcBandwidth(0.0, 0.0));
  }

  return 0;
}
