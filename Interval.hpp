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

#ifndef __ARSIM_INTERVAL_HPP__
#define  __ARSIM_INTERVAL_HPP__

#include "util.hpp"
#include <values.h>

#define INF MAXDOUBLE

/** An Interval instance represents an interval of reals, which may
 ** also be open on the left or right
 **/
class Interval {

 protected:

  double l, L;
  bool empty;

 public:

  Interval() { setEmpty(); }

  Interval(double min, double max) {
    if (min <= max)
      {  l = min;  L = max;  empty = false;  }
    else
      {  setEmpty();  }
  }

  void setEmpty() {  l = INF;  L = -INF;  empty = true;  }

  double getMin() const { return l; }
  double getMax() const { return L; }
  double getAvg() const { return ((l == -INF && L == INF) ? 0.0 : l == -INF ? -INF : L == INF ? INF : (l+L)/2.0); }

  bool isEmpty() const { return empty; }

  Interval intersect(const Interval& i) const {
    if (isEmpty() || i.isEmpty() || (L < i.l) || (l > i.L))
      return Interval();
    return Interval(MAX(l, i.l), MIN(L, i.L));
  }

  bool contains(double x) {  return ((! empty) && (l <= x) && (x <= L));  }

  Interval operator+(double delta) const {
    if (empty)
      return *this;
    return Interval(l == INF ? INF : l == -INF ? -INF : l + delta, L == INF ? INF : L == -INF ? -INF : L + delta);
  }
  Interval operator-(double delta) const {
    if (empty)
      return *this;
    return Interval(l == INF ? INF : l == -INF ? -INF : l - delta, L == INF ? INF : L == -INF ? -INF : L - delta);
  }
};

#endif
