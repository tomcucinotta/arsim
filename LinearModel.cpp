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
 * LinearModel.cpp
 *
 *  Created on: 10-mar-2008
 *      Author: tommaso
 */

#include "LinearModel.hpp"

#include <stdio.h>

LinearModel::LinearModel() : m(1.0), q(0.0) {  }

/** Build model given m and q parameters **/
LinearModel::LinearModel(double q, double m) : m(m), q(q) {  }

/** Set the model parameter q **/
void LinearModel::setQ(double new_q) { q = new_q; }

/** Get the model parameter q **/
double LinearModel::getQ() const { return q; }

/** Set the model parameter m **/
void LinearModel::setM(double new_m) { m = new_m; }

/** Get the model parameter m **/
double LinearModel::getM() const { return m; }

/** Compute simple fitting line: y = mx + q **/
void LinearModel::fit(double x1, double y1, double x2, double y2) {
  m = (y2 - y1) / (x2 - x1);
  q = y1 - m * x1;
}

/** Compute y corresponding to the supplied x value **/
double LinearModel::estimate(double x) {
  return m * x + q;
}

/** Compute x corresponding to the supplied y value **/
double LinearModel::estimateInv(double y) {
  return (y - q) / m;
}
