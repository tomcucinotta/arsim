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

#ifndef __ARSIM_UTIL_HPP__
#  define __ARSIM_UTIL_HPP__

#include <stdio.h>
#include <stdlib.h>

/** Debugging asserts: failure of cond is usually a programming error */
#define ASSERT(cond, msg) do { if (!(cond)) { fprintf(stderr, "ASSERT FAILED at line %d of file %s: %s\n", __LINE__, __FILE__, msg); abort(); } } while (0)
#define ASSERT1(cond, msg, param) do { if (!(cond)) { fprintf(stderr, "ASSERT FAILED: " msg "\n", param); abort(); } } while (0)

/** Unrecoverable check: failure of cond implies a run-time error to be notified to the user, and exit(-1) */
#define CHECK(cond, msg) do { if (!(cond)) { fprintf(stderr, "Error: %s\n", msg); exit(-1); } } while (0)
#define CHECK1(cond, msg, param) do { if (!(cond)) { fprintf(stderr, "Error: " msg "\n", param); exit(-1); } } while (0)
/** Boolean recoverable check: failure of cond implies a run-time error to be notified to the user */
#define BCHECK(cond, msg) do { if (!(cond)) { fprintf(stderr, "Error: %s\n", msg); return false; } } while (0)

#define MAX(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) ((a)<(b)?(a):(b))

#define DELTA 0.000001

namespace Logger {
#ifdef DEBUG_LOGGER
  void debugLog(const char *fmt, ...);
  void setLogFile(const char *fname);
  void close();
#else
  static inline void debugLog(const char *fmt, ...) {  }
  static inline void setLogFile(const char *fname) {  }
  static inline void close() {  }
#endif
}

#define UNASSIGNED -1

#endif
