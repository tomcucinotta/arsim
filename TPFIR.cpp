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

#include "TPFIR.hpp"
#include "util.hpp"

#include <math.h>

static double DEF_FILTER[] = {
  /* Optimum for flod4 */
 0.0593363234160615,
 0.0545330295574958,
 -0.0530111098169356,
 0.0207307512328371,
 0.0384090564092033,
 -0.038681469881062,
 -0.00417836647980184,
 0.0394173056331651,
 -0.0627037708969368,
 -0.0477852210897555,
 -0.0342979047577786,
 -0.093834376754735,
 -0.11022055850414,
 -0.136028648845247,
 0.517950571076614,
 0.00668662501790406,
 -0.0249614285117406,
 0.0088976758903979,
 0.0135379913550543,
 0.0185789476298815,
 -0.0632124155668821,
 0.0781471060616145,
 0.00708835104018764,
 -0.0855133171790548,
 -0.0665027580524265,
 -0.0391314913224437,
 0.0369186725369481,
 -0.0411506548453367,
 0.0534040779560707,
 0.0474445356976983,
 2.17420701248408e-05,
 -0.0537358500206691,
 0.108844884909586,
 -0.089208567424039,
 0.00192607322170356,
 -0.0101854234721098,
 -0.0627668293781823,
 -0.0853536672379242,
 0.349160914749155,
 -0.0180544579636103,
 -0.0852526596237162,
 -0.0173484583503196,
 -0.0228232386437533,
 -0.0197686350380224,
 0.0567930580670906,
 0.0632352716980729,
 0.0461844325576034,
 0.283838242520568,
 0.227983913205346,
 0.22643977270788,

 /* Mean of optmima for all traces */
//  0.0103047551437784,
//  0.0255338252085445,
//  -0.017794571792248,
//  0.00838186713385707,
//  -0.00066044203114745,
//  0.0302574017683895,
//  -0.000199171884803938,
//  0.0189088950198533,
//  -0.0423361686554374,
//  -0.023285904841143,
//  0.0141447641121654,
//  -0.101145944954658,
//  -0.0780894018570859,
//  -0.104528303449957,
//  0.389567006410486,
//  -0.0182108383779592,
//  -0.00826789227700213,
//  -0.013396091688972,
//  0.0103219821623684,
//  0.00203991768267441,
//  -0.0257293038404828,
//  -0.0133296047905398,
//  -0.0269349203329071,
//  -0.0177939149491289,
//  -0.0387392268333856,
//  -0.0292390998513653,
//  0.127440627512411,
//  -0.00779861907214474,
//  0.0173955530753572,
//  -0.00566265024834082,
//  -0.00181588649764264,
//  0.00767658613670337,
//  -0.00211993168941367,
//  -0.0122361978049858,
//  -0.0057410368092579,
//  -0.100577388403182,
//  -0.106796348334337,
//  -0.0989457553136248,
//  0.379416170256985,
//  0.00812720490307333,
//  -0.0137670059602869,
//  -9.25684695530926e-06,
//  -0.0109481515401409,
//  -0.0349743463432547,
//  0.0936785159585733,
//  0.0536529569482084,
//  0.0281101458753222,
//  0.269490610940629,
//  0.241474047197891,
//  0.224853082073664
};

TPFIR::TPFIR()
  : TPMoveableMean() {
  filter = DEF_FILTER;
  filter_size = sizeof(DEF_FILTER) / sizeof(DEF_FILTER[0]);
  setSampleSize(filter_size);
}

TPFIR::~TPFIR() {
}

bool TPFIR::parseArg(int& argc, char **& argv) {
  if (strcmp(*argv, "-fir-s") == 0) {
    CHECK(argc > 1, "Option requires an argument");
    argv++;  argc--;
    filter = new double[MAX_FIR_SIZE];
    ASSERT(filter != NULL, "No more memory");
    filter_size = 0;
    char *list = strdup(*argv);
    char *tmp;
    ASSERT(list != NULL, "No more memory");
    Logger::debugLog("TPFIR: analyzing FIR data string: %s\n", list);
    tmp = strtok(list, ",");
    while ((tmp != NULL) && (filter_size < MAX_FIR_SIZE)) {
      Logger::debugLog("TPFIR: found token: %s\n", tmp);
      CHECK(sscanf(tmp, "%lg", &filter[filter_size]) == 1, "Bad syntax for FIR samples");
      Logger::debugLog("TPFIR: read FIR sample: %lg\n", filter[filter_size]);
      filter_size++;
      tmp = strtok(NULL, ",");
    }
    free(list);
    CHECK(tmp == NULL, "Too many samples provided with -fir-s option");
    CHECK(filter_size > 0, "Unable to read FIR samples from -fir-s argument");
    setSampleSize(filter_size);
    Logger::debugLog("TPFIR: read FIR size: %d\n", filter_size);
  } else
    return parent::parseArg(argc, argv);
  return true;
}

void TPFIR::usage() {
  printf("(-tp fir)  -mm-s   Sample size for moving average\n");
  printf("(-tp fir)  -fir-s  Comma separated list of FIR samples (oldest to most recent)\n");
}

  /* Return expected next value */
double TPFIR::getExpValue() {
  if (q.size() == 0)
    return -1;
  double w_sum = 0.0;		// weighted sum of data samples
  double f_sum = 0.0;		// sum of filter samples
  int k = (int) MIN(filter_size, q.size());
  Logger::debugLog("Averaging %d samples with a queue size of %d\n", k, q.size());
  deque<double>::iterator it = q.end() - 1;
  for (int i=0; i < k; i++, it--) {
    ASSERT((filter_size - 1 - i >= 0) && (filter_size - 1 - i < filter_size), "Bad index");
    w_sum += (*it) * filter[filter_size - 1 - i];
    f_sum += filter[filter_size - 1 - i];
  }
  ASSERT(f_sum != 0.0, "Null-sum filter");
  return w_sum / f_sum;
}
