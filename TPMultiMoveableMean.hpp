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

#ifndef _TPMULTIMMEAN_H_
#  define _TPMULTIMMEAN_H_

#include "ValuePredictor.hpp"
#include "TPMoveableMean.hpp"

class TPMultiMoveableMean : public ValuePredictor {

 private:

  TPMoveableMean *p_tpreds;	/** Single predictors for various subsequences	*/
  int sub_period;		/** 12 is a typical value			*/
  int subseq;			/** Index, in p_tpreds[], of predictor to be used*/

public:

  static void usage();

  TPMultiMoveableMean();
  virtual ~TPMultiMoveableMean();

  virtual bool parseArg(int& argc, char **& argv);

  /** Add a c_k sample */
  virtual void addSample(double sample);
  /** Return expected next value */
  virtual double getExpValue() const;
//  /** Return standard deviation */
//  virtual double getDevValue();
//  /** Return positive deviation */
//  virtual double getDevPosValue();
//  /** Return negative deviation */
//  virtual double getDevNegValue();

  virtual void clearHistory();
};

#endif
