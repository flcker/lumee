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

#include "utils.h"

#include <cmath>
#include <sstream>

double scale_best_fit(int dest_width, int dest_height, int src_width,
    int src_height) {
  int w, h;
  if (src_width <= dest_width && src_height <= dest_height) {
    w = src_width;
    h = src_height;
  } else {
    w = dest_width;
    h = std::round(w * src_height / src_width);
    if (h > dest_height) {
      h = dest_height;
      w = std::round(src_width * h / src_height);
    }
  }
  return (double(w) / src_width + double(h) / src_height) / 2;
}

std::string to_percentage(double decimal) {
  std::stringstream s;
  s << std::round(decimal * 100) << "%";
  return s.str();
}
