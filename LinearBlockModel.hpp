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
 * LinearBlockModel.hpp
 *
 *  Created on: 10-mar-2008
 *      Author: tommaso
 */

#ifndef LINEARBLOCKMODEL_HPP_
#define LINEARBLOCKMODEL_HPP_

#include "LinearModel.hpp"

#include <vector>

#include "util.hpp"

/** Implementation of a linear block model:
 ** y = ceil(mx + q, h)*h + k
 ** y <= (mx+q) + h + k ==>  x >= (y - h - k - q)/m
 **/
class LinearBlockModel : public LinearModel {
protected:
  double h; ///< The block-size (granularity of ceiling)
  double k; ///< Further quantity to sum-up after ceiling
public:
  /** Build an identity model (y = x), mainly useful for fitting it afterwards **/
  LinearBlockModel();
  /** Build model given m and q parameters **/
  LinearBlockModel(double q, double m, double h, double k);
  /** Compute y corresponding to the supplied x value **/
  virtual double estimate(double x);
  /** Compute upper bound of x corresponding to the supplied y value **/
  virtual double estimateInv(double y);
};

#endif /* LINEARBLOCKMODEL_HPP_ */
