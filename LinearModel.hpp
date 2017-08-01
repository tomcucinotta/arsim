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
 * LinearModel.hpp
 *
 *  Created on: 10-mar-2008
 *      Author: tommaso
 */

#ifndef LINEARMODEL_HPP_
#define LINEARMODEL_HPP_

#include "Model.hpp"

#include <vector>

#include "util.hpp"

/** Implementation of a simple linear model: y = mx + q **/
class LinearModel : public Model {
protected:
  double m, q;
public:
  /** Build an identity model (y = x), mainly useful for fitting it afterwards **/
  LinearModel();
  /** Build model given m and q parameters **/
  LinearModel(double q, double m);
  /** Set the model parameter q **/
  void setQ(double q);
  /** Get the model parameter q **/
  double getQ() const;
  /** Set the model parameter m **/
  void setM(double m);
  /** Get the model parameter m **/
  double getM() const;
  /** Compute simple fitting line: y = m * x + q **/
  void fit(double x1, double y1, double x2, double y2);
  /** Compute best fitting line in the least mean square error sense: min { sum_i (y[i] - m * x[i] + q)^2 } **/
  template<class x_iterator, class y_iterator>
  void fit(x_iterator & x_it, x_iterator & x_it_end,
      y_iterator & y_it, y_iterator & y_it_end) {
    double sum_xy = 0, sum_x = 0, sum_y = 0, sum_x2 = 0;
    CHECK(x_it_end - x_it == y_it_end - y_it, "Vectors (segments) to fit must be equally long");
    for (; x_it != x_it_end; ++x_it, ++y_it) {
      sum_xy += (*x_it) * (*y_it);
      sum_x += (*x_it);
      sum_y += (*y_it);
      sum_x2 += (*x_it) * (*x_it);
    }
    m = (sum_xy - sum_x * sum_y) / (sum_x2 - sum_x * sum_x);
    q = sum_y - m * sum_x;
  }
  /** Compute y corresponding to the supplied x value **/
  virtual double estimate(double x);
  /** Compute x corresponding to the supplied y value **/
  virtual double estimateInv(double y);
};

#endif /* LINEARMODEL_HPP_ */
