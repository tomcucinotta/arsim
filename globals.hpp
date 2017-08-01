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

#ifndef __ARSIM_GLOBALS_HPP__
#define   __ARSIM_GLOBALS_HPP__

/** Types of exit condition that may cause the end of simulation */
typedef enum {
  XC_TIME,		/**< Exit at specified virtual time	*/
  XC_JOB		/**< Exit at specified job termination	*/
} ExitCond;

extern ExitCond exit_cond;	/**< Exit condition			*/
extern double x_time;		/**< Virtual time of exit		*/
extern long int x_job;		/**< Last Job Id to execute		*/
extern int x_rs;		/**< Resource to which last job belongs	*/
extern int x_tsk;		/**< Task to which last job belongs	*/

#endif
