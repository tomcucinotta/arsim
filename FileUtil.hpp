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

#ifndef FILEUTIL_HPP_
#define FILEUTIL_HPP_

#include <vector>

/** Read the specified column of numbers into samples from the file, discarding the commented
 ** lines and the first disc_lines lines. Also, multiply all read samples by mul_factor.
 ** 
 ** Returns the number of correctly read lines, i.e. the size of samples after being filled.
 **/
long loadTrace(std::vector<double> & samples, const char * trace_fname, long disc_lines, int col_number, double mul_factor);

/** Read a comma-separated list of real samples, adding them to the supplied vector.
 **
 ** Return the number of read samples.
 **/
int parseList(char *ptr, std::vector<double> & samples);

/** Split the string c_str in corrispondence of spaces.
 **
 ** The supplied input string c_str is modified, replacing delimiting
 ** characters (space) with null string terminators.
 **
 ** Each element of the supplied argv[] vector is made to point to a
 ** segment of c_str.  The argv[] vector needs to be sufficiently
 ** large to store all the identified arguments in c_str, or an
 ** assertion is raised.
 **
 ** @return The number of actually parsed arguments (argc) */
int stringToArgv(char *c_str, char **argv, int argv_size);

#endif /* FILEUTIL_HPP_ */
