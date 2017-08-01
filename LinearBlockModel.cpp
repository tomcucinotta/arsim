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

/*
 * LinearBlockModel.cpp
 *
 *  Created on: 10-mar-2008
 *      Author: tommaso
 */

#include "LinearBlockModel.hpp"

#include <stdio.h>
#include <math.h>

LinearBlockModel::LinearBlockModel() : LinearModel(), h(1.0), k(0.0) {  }

LinearBlockModel::LinearBlockModel(double q, double m, double h, double k) :
  LinearModel(m, q), h(h), k(k) {  }

/** Compute y corresponding to the supplied x value **/
double LinearBlockModel::estimate(double x) {
  return ceil((m * x + q) / h) * h;
}

/** Compute upper bound of x corresponding to the supplied y value **/
double LinearBlockModel::estimateInv(double y) {
  return (y - h - k - q) / m;
}
