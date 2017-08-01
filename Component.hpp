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

#ifndef COMPONENT_HPP_
#define COMPONENT_HPP_

/** Base interface for simulator components, enabling argument parsing
 ** and checking of supplied parameters before simulation actually starts.
 **/
class Component {

 public:

  /** Parse command-line arguments.
   **
   ** Return true if any arguments recognized (and argc/argv accordingly
   ** advanced so as to skip the recognized arguments).
   **/
  virtual bool parseArg(int& argc, char **& argv) { return false; }
  virtual bool checkParams() {  return true; }
  virtual void calcParams() {  }
  virtual ~Component() { };
};

#endif /*COMPONENT_HPP_*/
