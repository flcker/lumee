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

#ifndef LUMEE_UTILS_H
#define LUMEE_UTILS_H

#include <gdkmm/rectangle.h>
#include <gdkmm/pixbuf.h>

// Information about the application's runtime environment.
class RuntimeInfo {
 public:
  // Must be called first.
  static void init();

  static bool is_installed() { return installed; }
  static std::string get_data_dir() { return data_dir; }

 private:
  static bool installed;
  static std::string data_dir;
};

// Represents two dimensions. It can be used to fit one area into another; see
// the `fit()` member function. Example:
//
//     double factor = Dimensions(pixbuf).fit(allocation);
class Dimensions {
 public:
  Dimensions(int width, int height) : width(width), height(height) {}
  Dimensions(int square) : Dimensions(square, square) {}
  Dimensions(const Gdk::Rectangle& rectangle)
      : Dimensions(rectangle.get_width(), rectangle.get_height()) {}
  Dimensions(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
      : Dimensions(pixbuf->get_width(), pixbuf->get_height()) {}

  // Returns the scale factor needed to fit these dimensions into the target
  // dimensions. The factor won't exceed 1.0 unless `expand` is true.
  //
  // If `scrollbar_width` is specified, the scaled height will be allowed to
  // exceed the target height, with the scrollbar's width being subtracted from
  // the target width in this case.
  double fit(Dimensions target, bool expand = false, int scrollbar_width = 0)
      const;

  int width = 0;
  int height = 0;
};

// Point represented by two coordinates.
struct Point {
  Point(double x, double y) : x(x), y(y) {}
  Point() {}

  double x = 0.0;
  double y = 0.0;
};

// Converts a decimal to its percentage (for example, 0.12 becomes "12%").
inline std::string to_percentage(double decimal) {
  std::stringstream s;
  s << std::round(decimal * 100) << "%";
  return s.str();
}

// Wrapper for `g_utf8_collate_key_for_filename()`.
inline std::string collate_key_for_filename(const Glib::ustring& filename) {
  return Glib::convert_return_gchar_ptr_to_stdstring(
      g_utf8_collate_key_for_filename(filename.data(), filename.size()));
}

#endif
