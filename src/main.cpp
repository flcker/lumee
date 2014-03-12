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

#include "application.h"

#include <glibmm/miscutils.h>

// Support C++11 lambdas as sigc functors.
namespace sigc { SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE }

int main(int argc, char* argv[]) {
  RuntimeInfo::init();
  if (!RuntimeInfo::is_installed())
    Glib::setenv("GSETTINGS_SCHEMA_DIR", RuntimeInfo::get_data_dir());
  return Application::create()->run(argc, argv);
}
