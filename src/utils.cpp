// Copyright (C) 2014 Brian Marshall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "utils.h"

#include <glibmm/miscutils.h>

#include <climits>
#include <cmath>
#include <sstream>
#include <unistd.h>

bool RuntimeInfo::installed = true;
std::string RuntimeInfo::data_dir = PKGDATADIR;

// Checks the location of the current executable, in order to support being run
// from the source tree. Doing this isn't portable and may need changes for
// other operating systems.
//
// static
void RuntimeInfo::init() {
  char exe_file[PATH_MAX];
  ssize_t size = readlink("/proc/self/exe", exe_file, PATH_MAX);
  if (size != -1) {
    std::string exe_dir = Glib::path_get_dirname(std::string(exe_file, size));
    if (exe_dir != BINDIR) {
      installed = false;
      data_dir = Glib::build_filename(exe_dir, "data");
    }
  }
}

// When `scrollbar_width` is nonzero, only width is constrained. Otherwise,
// both width and height are constrained.
double Dimensions::fit(Dimensions target, bool expand, int scrollbar_width)
    const {
  if (!expand && width <= target.width &&
      (height <= target.height ||
       (scrollbar_width && width <= target.width - scrollbar_width)))
    return 1.0;

  double w = target.width;
  double h = w * height / width;
  if (std::round(h) > target.height) {
    if (scrollbar_width) {
      w = target.width - scrollbar_width;
      h = w * height / width;
    }
    // Sometimes the image is no longer tall enough to cause a vertical
    // scrollbar after subtracting the scrollbar width above.
    if (!scrollbar_width || std::round(h) <= target.height) {
      h = target.height;
      w = width * h / height;
    }
  }
  return (w / width + h / height) / 2;
}

std::string to_percentage(double decimal) {
  std::stringstream s;
  s << std::round(decimal * 100) << "%";
  return s.str();
}
