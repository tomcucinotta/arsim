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

#ifndef __ARSIM_DEFAULTS_HPP__
#  define  __ARSIM_DEFAULTS_HPP__

/** Task defaults */

#define DEF_h	1.3	/** Default full range excursion minimum		*/
#define DEF_H	8.2	/** Default full range excursion maximum		*/

/** Spike task defaults */

#define DEF_P_FR 0.1	/** Default full range excursion probability	*/

/** TriSpike task defaults */

#define DEF_h_w	 0.125	/** Default internal range semi-excursion	*/
#define DEF_tpe  0.4	/** Default triangular pipe semi-excursion	*/


/** TraceTask defaults */

#define DEF_TFNAME "trace.dat"

/** General scheduler defaults */

#define DEF_T		40.0	/**< Default task period	*/
#define DEF_Ts		0.5	/**< Default server period	*/
#define DEF_SCHED  "la"		/**< Default scheduler		*/
#define DEF_PREDICTOR "mm"	/**< Default predictor type	*/
#define DEF_SRV_PER 5000	/**< Default server period	*/

/** LimitedController defaults */

#define DEF_BW_MAX 1.0	/** Maximum b(k) value			*/
#define DEF_BW_MIN 0.0	/** Minimum guaranteed b(k) value	*/
#define DEF_P   0.5	/** Default server period in discrete model     */
#define DEF_g   0.8	/** Exponential reduction factor		*/

/** DoubleLimited Controller defaults	*/
#define DEF_DL_e0 0

/**  Optimal Cost (OC) scheduler defaults */
#define DEF_OC_GAMMA 0.75

/**  PIScheluder scheduler defaults	*/
#define DEF_PI_Z1 0.01
#define DEF_PI_Z2 0.4
#define DEF_PI_EPS_TARGET 0.0

/** eps(k) stat parameters		*/
#define DEF_EPS_DX  (0.010)
//#define DEF_EPS_DX  (0.001)
#define DEF_EPS_MIN (-1.0)
#define DEF_EPS_MAX (4.0)

/** Global Optimizer defaults		*/
/** Default load for optimiz. problem	*/
#define DEF_AVG_LOAD (0.3)

#endif
