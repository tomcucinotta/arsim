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

#ifndef _TASK_PREDICTOR_H_
#  define _TASK_PREDICTOR_H_

#include "Interval.hpp"
#include "Component.hpp"
#include "ValuePredictor.hpp"
#include "RangePredictor.hpp"

#include <deque>

using namespace std;

/** This class represents a generic predictor for a task working times.
 **
 ** The static getInstance() method allows for instantiation of any
 ** existing subclass.
 **/
class TaskPredictor : public Component {

 protected:

  ValuePredictor *p_vpred; //*< Task computation time value predictor
  RangePredictor *p_rpred; //*< Task computation time range predictor
  RangePredictor *p_rpred_i; //*< Task computation time internal range predictor
  bool stack_rp;           //*< RangePredictor stacked after ValuePredictor

  deque<double> q;
  double sum;
  double sum_sqr;
  int sample_size;

  RangePredictor *p_curr_rpred; //*< Current range predictor, for parsing cmd-line options */

  /** Enable perfect prediction                 */
  bool perfect_pred;
  /** Enable perfect prediction specific range alpha */
  double perfect_pred_alpha;
  /** Perfectly predicted (i.e. next) sample    */
  double perfect_pred_sample;

 public:

  TaskPredictor();

  static void usage();
  virtual bool parseArg(int& argc, char **& argv);
  void calcParams();

  /** Add a c_k sample */
  virtual void addSample(double sample);
  /** Return range in which next sample would reside with high probability */
  virtual Interval getExpInterval() const;
  /** Return smaller range in which next sample would reside with lower probability */
  virtual Interval getExpIntervalI() const;
  /** Return expected next value */
  virtual double getExpValue() const;
  /** Return c_k standard deviation */
  virtual double getDevValue() const;
//  /** Return positive deviation */
//  virtual double getDevPosValue();
//  /** Return negative deviation */
//  virtual double getDevNegValue();

  /** Provide next sample to be predicted.
   **
   ** A ValuePredictor may just return the supplied sample at
   ** the very next getExpValue() call.
   ** A RangePredictor should ensure the predicted range at
   ** the very next getExpInterval() and getExpIntervalI() calls
   ** will contain the supplied sample.
   **/
  void setPerfectPrediction(double sample) {
    perfect_pred_sample = sample;
  }

  virtual void clearHistory();
};

#endif
