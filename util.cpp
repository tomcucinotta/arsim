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

#include "util.hpp"
#include <stdarg.h>
#include <string.h>
#include <zlib.h>

#ifdef DEBUG_LOGGER

FILE *log_file = stderr;
gzFile gz_log_file = NULL;

/** If fname has a ".gz" suffix, then logfile is automatically compressed. **/
void Logger::setLogFile(const char *fname) {
  if (log_file != stderr)
    fclose(log_file);
  if (strlen(fname) >= 3 && strcmp(fname + strlen(fname) - 3, ".gz") == 0) {
    log_file = NULL;
    gz_log_file = gzopen(fname, "w");
    ASSERT1(gz_log_file != NULL, "Couldn't open log file '%s' !\n", fname);
  } else {
    log_file = fopen(fname, "w");
    ASSERT1(log_file != NULL, "Couldn't open log file '%s' !\n", fname);
  }
}

char line_buf[1024];

void Logger::debugLog(const char *fmt, ...) {
  va_list val;
  va_start(val, fmt);
  if (log_file != NULL) {
    vfprintf(log_file, fmt, val);
    fflush(log_file);
  } else if (gz_log_file != NULL) {
    vsnprintf(line_buf, sizeof(line_buf), fmt, val);
    gzputs(gz_log_file, line_buf);
    // gzflush(gz_log_file);    // Can degrade compression
  }
}

void Logger::close() {
  if (log_file != NULL) {
    fclose(log_file);
  } else if (gz_log_file != NULL) {
    gzclose(gz_log_file);
  }
}

#endif
