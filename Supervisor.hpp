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

#ifndef SUPERVISOR_HPP_
#define SUPERVISOR_HPP_

#include "TaskScheduler.hpp"

/**
 * Supervisor base class.
 * 
 * The supervisor is responsible for assigning the correct
 * bandwidth to the tasks by calling the method
 * tasks[i]->setCurrentBandwidth(), in accordance to the values
 * required by them, which is retrieved through the method
 * tasks[i]->getRequiredBandwidth().
 */

class Supervisor : public Component {
  double speed;                 /**< Resource current speed     */
  vector<TaskScheduler*> *p_tasks;
public:
  Supervisor();
  virtual void checkGlobalConstraint(vector<TaskScheduler*>& tasks) = 0;
  static Supervisor * getInstance(const char * name);
  static void usage();
  virtual bool parseArg(int& argc, char **& argv);
  virtual bool checkParams();
  virtual void dumpStats() { }
  virtual ~Supervisor() { }

  void setTasks(vector<TaskScheduler*> *p_tasks) {
    this->p_tasks = p_tasks;
  }

  /** Get Resource Speed (power management) */
  double getSpeed() const { return speed; }

  /** Set Resource Speed (power management).
   ** If minimum guaranteed bws are changed together with resource speed,
   ** be sure it is done BEFORE the setSpeed() call
   **/
  void setSpeed(double new_speed) {
    speed = new_speed;
    // Supervisor checks if bandwidths may still be granted after speed change
    Logger::debugLog("Checking global constraints after speed set to: %g\n", new_speed);
    checkGlobalConstraint(*p_tasks);
  }

};

#endif /*SUPERVISOR_HPP_*/
