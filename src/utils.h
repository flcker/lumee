/* Copyright (C) 2014 Brian Marshall
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUMEE_UTILS_H
#define LUMEE_UTILS_H

#include <string>

// Returns the scale factor needed to fit the source area into the destination
// area with aspect ratio preserved.
double scale_best_fit(int dest_width, int dest_height, int src_width,
    int src_height);

// Converts a decimal to its percentage (for example, 0.12 becomes "12%").
std::string to_percentage(double decimal);

// Returns a path to the data directory.
std::string get_data_dir();

#endif
