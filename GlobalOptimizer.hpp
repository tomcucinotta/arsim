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

#ifndef __GLOBAL_CONTROLLER__
#define __GLOBAL_CONTROLLER__

#include "Component.hpp"
#include "ResourceManager.hpp"

#include "qos_opt.h"
#include "Events.hpp"

/** Global QoS Controller: optimizes global QoS base on a global vision of the system
 **
 ** @TODO Implement application mode within Task::setAppMode()
 **
 ** @see Task::setAppMode
 **/
class GlobalOptimizer : public Component {
  const char * opt_type;
  qos_opt *p_opt;
  int na, nam, nr, nrm;
  int use_mmp;
  static GlobalOptimizer *p_gc;
  Event *p_opt_ev;
  long unsigned opt_period; //< If different from zero, then enable global optimization
  FILE *gc_file;
  Stat obj_val_stat;
  Stat perf_index_stat;

public:

  GlobalOptimizer();
  virtual bool parseArg(int& argc, char **& argv);
  virtual bool checkParams();
  static void usage();
  void dumpStatistics();
  static inline GlobalOptimizer *getInstance() {
    return p_gc;
  }
  /** Optimize globally the system QoS by reconfiguring application modes,
   ** resource power modes and minimum guaranteed bandwidths                    **/
  void optimize();
  void handleUpdateBandEvent(const Event & ev);
  virtual ~GlobalOptimizer();
  double getOptPeriod() const { return opt_period; }
};

#endif
