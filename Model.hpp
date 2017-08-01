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
 * Model.hpp
 *
 *  Created on: 10-mar-2008
 *      Author: tommaso
 */

#ifndef MODEL_HPP_
#define MODEL_HPP_

/** Basic model virtual interface **/
class Model {
public:
  /** Compute y corresponding to the supplied x value **/
  virtual double estimate(double x) = 0;
  /** Compute x corresponding to the supplied y value **/
  virtual double estimateInv(double y) = 0;
};

#endif /* MODEL_HPP_ */
