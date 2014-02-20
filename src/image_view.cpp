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

#include "image_view.h"
#include "utils.h"

const double ImageView::ZOOM_MIN = 0.15;
const double ImageView::ZOOM_MAX = 3.0;

ImageView::ImageView(BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder) : Gtk::ScrolledWindow(cobject) {
  builder->get_widget("image", image);
}

void ImageView::set(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf) {
  this->pixbuf = pixbuf;
  prev_zoom_factor = 0.0;
  show_image();
}

void ImageView::clear() {
  pixbuf.reset();
  image->clear();
}

void ImageView::show_image() {
  if (!pixbuf)
    return;
  if (zoom_mode == ZOOM_BEST_FIT)
    zoom_factor = scale_best_fit(get_allocated_width(), get_allocated_height(),
        pixbuf->get_width(), pixbuf->get_height());
  zoom_factor = std::max(ZOOM_MIN, std::min(ZOOM_MAX, zoom_factor));
  if (zoom_factor == prev_zoom_factor)
    return;  // Zoom factor didn't change, so there's no need to re-scale.
  image->set(zoom_factor == 1.0 ? pixbuf : pixbuf->scale_simple(
        std::round(pixbuf->get_width() * zoom_factor),
        std::round(pixbuf->get_height() * zoom_factor), Gdk::INTERP_BILINEAR));
  prev_zoom_factor = zoom_factor;
}
