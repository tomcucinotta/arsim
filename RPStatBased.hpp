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

#ifndef _RP_STAT_BASED_H_
#  define _RP_STAT_BASED_H_

#include "TaskPredictor.hpp"

#include <deque>

using namespace std;

class RPStatBased : public RangePredictor {

protected:

  std::deque<double> q;
  int sample_size;              /**< For estimating moveable mean and dev of c(k) */
  double percentile;            /**< Percentile at which to compute range         */

public:

  static void usage();

  RPStatBased();
  virtual bool parseArg(int& argc, char **& argv);

  /** Add a c_k sample */
  virtual void addSample(double sample);
  /** Return range in which next sample would reside with high probability */
  virtual Interval getExpInterval();

  /** Set Sample Size from outside */
  void setSampleSize(int new_size);

  /** Set Percentile from outside */
  void setPercentile(double new_perc);
  /** Get Percentile from outside */
  double getPercentile() const { return percentile; }

  virtual void clearHistory();
};

#endif
