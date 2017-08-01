#include "Task.hpp"

class TaskFactory {
public:
  static Task * getInstance(const char * s);
  static void usage();
};
