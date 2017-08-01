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

#ifndef _TP_FIR_H_
#  define _TP_FIR_H_

#include "TPMoveableMean.hpp"

#define MAX_FIR_SIZE 256

using namespace std;

class TPFIR : public TPMoveableMean {

protected:

  double *filter;
  unsigned int filter_size;

public:

  typedef TPMoveableMean parent;

  static void usage();

  TPFIR();
  virtual ~TPFIR();
  virtual bool parseArg(int& argc, char **& argv);

  /** Return expected next value */
  virtual double getExpValue();
};

#endif
