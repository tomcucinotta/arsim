#include <TaskFactory.hpp>

#include "StableTask.hpp"
#include "TriangleTask.hpp"
#include "SpikeTask.hpp"
#include "TriSpikeTask.hpp"
#include "TraceTask.hpp"
#include "CmdlineTask.hpp"
#include "MultiModeTask.hpp"
#include "TPMoveableMean.hpp"
#include "RobotControlTask.hpp"

#include <string.h>

Task * TaskFactory::getInstance(const char * s) {
  if (strcmp(s, "u") == 0)
    return new Task();
  else if (strcmp(s, "s") == 0)
    return new StableTask();
  else if (strcmp(s, "t") == 0)
    return new TriangleTask();
  else if (strcmp(s, "p") == 0)
    return new SpikeTask();
  else if (strcmp(s, "tp") == 0)
    return new TriSpikeTask();
  else if (strcmp(s, "tr") == 0)
    return new TraceTask();
  else if (strcmp(s, "cl") == 0)
    return new CmdlineTask();
  else if (strcmp(s, "mmt") == 0)
    return new MultiModeTask();
  else if (strcmp(s, "rct") == 0)
    return new RobotControlTask();
  return 0;
}

void TaskFactory::usage() {
  DoubleLimitedTask::usage();
  StableTask::usage();
  SpikeTask::usage();
  TriangleTask::usage();
  TraceTask::usage();
  CmdlineTask::usage();
}
