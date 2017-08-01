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

#include <cstring>

#include "TPMultiMoveableMean.hpp"

#include "util.hpp"

TPMultiMoveableMean::TPMultiMoveableMean() {
  sub_period = 12;
  p_tpreds = new TPMoveableMean[sub_period];
  ASSERT(p_tpreds != 0, "No more memory !");
  subseq = 0;
}

void TPMultiMoveableMean::clearHistory() {
  for (int i = 0; i < sub_period; ++i)
    p_tpreds[i].clearHistory();
  subseq = 0;
}

TPMultiMoveableMean::~TPMultiMoveableMean() {
  delete[] p_tpreds;
}

bool TPMultiMoveableMean::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-mmm-sp") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    sub_period = atoi(*argv);
    CHECK(sub_period > 0, "Expecting a positive integer as argument to -mmm-sp option");
    delete[] p_tpreds;
    p_tpreds = new TPMoveableMean[sub_period];
    ASSERT(p_tpreds != 0, "No more memory !");
  } else if (strcmp(*argv, "-mmm-ss") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    int sample_size = atoi(*argv);
    CHECK(sample_size > 0, "Expecting a positive integer as argument to -mmm-ss option");
    for (int i=0; i<sub_period; i++)
      p_tpreds[i].setSampleSize(sample_size);
  } else
    return false;
  return true;
}

void TPMultiMoveableMean::usage() {
  printf("(-tp mumm) -mmm-sp Sub period for multiple moving averages\n");
  printf("(-tp mumm) -mmm-ss Sample size for each average\n");
}

void TPMultiMoveableMean::addSample(double sample) {
  // Logger::debugLog("Adding %g to seq n.%d\n", sample, subseq);
  p_tpreds[subseq].addSample(sample);
  subseq = (subseq + 1) % sub_period;
}

  /* Return expected next value */
double TPMultiMoveableMean::getExpValue() const {
  return p_tpreds[subseq].getExpValue();
}

//  /* Return range in which next sample would reside with high probability */
//Interval TPMultiMoveableMean::getExpInterval() {
//  return p_tpreds[subseq].getExpInterval();
//}
//
//  /* Return standard deviation */
//double TPMultiMoveableMean::getDevValue() {
//  return p_tpreds[subseq].getDevValue();
//}
//
//  /* Return positive deviation */
//double TPMultiMoveableMean::getDevPosValue() {
//  return p_tpreds[subseq].getDevPosValue();
//}
//
//  /* Return negative deviation */
//double TPMultiMoveableMean::getDevNegValue() {
//  return p_tpreds[subseq].getDevNegValue();
//}
