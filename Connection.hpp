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

#ifndef __ARSIM_CONNECTION_HPP__
#  define __ARSIM_CONNECTION_HPP__

#include "Task.hpp"

/** This class represents a reserved network connection (virtual link)
 ** among computing tasks.
 ** 
 ** The infrastructure for networking is similar to the one for
 ** computing: many connections may share the same physical link, and
 ** feedback-based scheduling may be performed independently on each
 ** connection. Therefore, a supervisor is needed whenever the actual
 ** capacity of the physical link is saturated.
 **
 ** As of now, Connection distinguishes from Task simply for the
 ** explicit consideration of Security policies possibly in action
 ** on the link, for which a precise formula is used to transform
 ** the bandwidth occupation of the application payload to the one
 ** of the secured traffic.
 **/
class Connection : public Task {

 private:

  long int block_size;

 public:

  Connection();
  /** Return next task instance time c(k) */
  virtual double generateInstance();

  static void usage();
};

#endif
