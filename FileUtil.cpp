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
#include "FileUtil.hpp"
#include "util.hpp"

#include <iostream>
#include <sstream>

/** Read the specified column of numbers into samples from the file, discarding the commented
 ** lines and the first disc_lines lines.
 ** 
 ** Returns the number of correctly read lines.
 **/
long loadTrace(std::vector<double> & samples, const char * trace_fname, long disc_lines, int col_number, double mul_factor) {
  FILE *ff;
  printf("# Loading column %d of trace file '%s' scaled by %g, discarding %ld lines\n", col_number, trace_fname, mul_factor, disc_lines);
  ff = fopen(trace_fname, "r");
  ASSERT(ff != 0, "Couldn't find task trace file");

  char line_buf[256];
  long int line_num = 0;
  for (int i = 0; i < disc_lines && ! feof(ff); ++i) {
    if (fgets(line_buf, sizeof(line_buf), ff) == NULL)
      continue;
    ++line_num;
  }

  while (! feof(ff)) {
    double sample;
    if (fgets(line_buf, sizeof(line_buf), ff) == NULL)
      continue;
    ++line_num;
    if (strlen(line_buf) == 0) {
      fprintf(stderr, "Warning: skipping empty line %ld\n", line_num);
      continue;
    }
    while (strlen(line_buf) > 0
        && (line_buf[strlen(line_buf) - 1] == 0x0A || line_buf[strlen(line_buf) - 1] == 0x0D))
              line_buf[strlen(line_buf) - 1] = '\0';
    Logger::debugLog("Scanning line %ld: '%s'\n", line_num, line_buf);
    if (strlen(line_buf) > 0 && line_buf[0] == '#') {
      Logger::debugLog("Warning: skipping commented line %ld: '%s'\n", line_num, line_buf);
      continue;
    }
    std::istringstream iss(line_buf);
    bool sample_valid = true;
    std::string sample_string;
    for (int i = 0; i < col_number - 1; ++i) {
      if (iss.eof()) {
        sample_valid = false;
        break;
      } else
        iss >> sample_string;
    }
    if (sample_valid) {
      if (iss.eof())
        sample_valid = false;
      else
        iss >> sample_string;
      if (sample_valid && sscanf(sample_string.c_str(), "%lf", &sample) != 1)
        sample_valid = false;
    }
    if (sample_valid) {
      double actual_sample = sample * mul_factor;
      //Logger::debugLog("Adding sample %g reworked as %g\n", sample, actual_sample);
      samples.push_back(actual_sample);
    } else {
      fprintf(stderr, "Warning: skipping line %ld '%s': column %d not numeric\n", line_num, line_buf, col_number);
    }
  }
  fclose(ff);
  printf("# Loaded %d samples\n", (int) samples.size());
  return samples.size();
}

int parseList(char *ptr, std::vector<double> & samples) {
  char *endptr;
  int num = -1;
  double sample;
  do {
    sample = strtod(ptr, &endptr);
    CHECK(endptr != ptr, "Expecting comma-separated list of doubles");
    //Logger::debugLog("Adding sample: %lg\n", sample);
    samples.push_back(sample);
    CHECK(*endptr == '\0' || *endptr == ',', "Expecting comma-separated list of doubles");
    ptr = endptr + 1; // Skip comma or null-terminator
    ++num;
  } while (*endptr != '\0');
  return num;
}

int stringToArgv(char *c_str, char **argv, int argv_size)
{
  int argc = 0;
  char *saveptr;
  char *arg = strtok_r(c_str, " ", &saveptr);
  while (arg != NULL) {
    // Otherwise the supplied buffer was short and we lost one or more args
    CHECK(argc < argv_size, "Need a larger argv[]");
    argv[argc++] = arg;
    arg = strtok_r(NULL, " ", &saveptr);
  }
  return argc;
}
