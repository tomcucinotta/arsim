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

#include "ValuePredictor.hpp"

#include "TPMoveableMean.hpp"
#include "TPMultiMoveableMean.hpp"
#include "TPStaticValue.hpp"
#include "TPFIR.hpp"
#include "TPOptimumLinearFIR.hpp"
#include "TPValueTrace.hpp"

ValuePredictor * ValuePredictor::getInstance(const char *s) {
  if (strcmp(s, "mm") == 0)
    return new TPMoveableMean();
  else if (strcmp(s, "mumm") == 0)
    return new TPMultiMoveableMean();
  else if (strcmp(s, "sv") == 0)
    return new TPStaticValue();
  else if (strcmp(s, "fir") == 0)
    return new TPFIR();
  else if (strcmp(s, "ol") == 0)
    return new TPOptimumLinearFIR();
  else if (strcmp(s, "tvp") == 0)
    return new TPValueTrace();
  return 0;
}

bool ValuePredictor::parseArg(int& argc, char **& argv) {
  return false;
}

void ValuePredictor::usage() {
  printf("    VALUE PREDICTOR OPTIONS\n");
  TPStaticValue::usage();
  TPMoveableMean::usage();
  TPMultiMoveableMean::usage();
  TPFIR::usage();
  TPOptimumLinearFIR::usage();
}
