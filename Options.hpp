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

#ifndef __ARSIM_OPTIONS_HPP__
#  define __ARSIM_OPTIONS_HPP__

typedef enum { OPT_INT, OPT_DBL, OPT_STR } OptionType;

template <class T>
class Option {

private:

  const char * opt_switch;
  const char * opt_descr;
  OptionType opt_type;
  union {
    int * p_int;
    double * p_dbl;
    char *p_str;
  };

public:

  Option(const char * swtch, const char * descr, double *p) {
    opt_switch = swtch;
    opt_descr = descr;
    opt_type = OPT_DBL;
    p_dbl = p;
  }
  Option(const char * swtch, const char * descr, int *p) {
    opt_switch = swtch;
    opt_descr = descr;
    opt_type = OPT_INT;
    p_int = p;
  }
  Option(const char * swtch, const char * descr, char *p) {
    opt_switch = swtch;
    opt_descr = descr;
    opt_type = OPT_STR;
    p_str = p;
  }
  OptionType getType() const { return opt_type; }
  const char * getSwitch() const { return opt_switch; }
  const char * getDescription() const { return opt_descr; }
  int * getPInt() const { return p_int; }
  double * getPDouble() const { return p_dbl; }
  char * getPString() const { return p_str; }
};

#endif
