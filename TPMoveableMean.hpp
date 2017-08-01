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

#ifndef _TP_MMEAN_H_
#  define _TP_MMEAN_H_

#include "ValuePredictor.hpp"

#include <deque>

using namespace std;

class TPMoveableMean : public ValuePredictor {

 protected:

  std::deque<double> q;
  int sample_size;	/** For estimating moveable mean and dev of c(k) */
  double sum;
  bool discard_old_samples;	/** Enable discarding of old samples	*/

public:

  static void usage();

  TPMoveableMean();
  virtual bool parseArg(int& argc, char **& argv);

  /** Add a c_k sample */
  virtual void addSample(double sample);
  /** Return expected next value */
  virtual double getExpValue() const;

  /** Set Sample Size from outside */
  void setSampleSize(int new_size);

  void clearHistory();
};

#endif
