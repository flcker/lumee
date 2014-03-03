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

#include <gtkmm/scrollbar.h>

const std::vector<double> ImageView::ZOOM_STEPS = {0.15, 1/3.0, 0.5, 2/3.0,
    1.0, 1.5, 2.0, 2.5, 3.0};
const double ImageView::ZOOM_MULTIPLIER = 1.1;
const double ImageView::ZOOM_MIN = ZOOM_STEPS.front();
const double ImageView::ZOOM_MAX = ZOOM_STEPS.back();

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

void ImageView::zoom_in(bool step) {
  if (step) {
    auto iter = std::find_if(ZOOM_STEPS.begin(), ZOOM_STEPS.end(),
        [this](double x) { return x > zoom_factor; });
    zoom_to(iter != ZOOM_STEPS.end() ? *iter : ZOOM_MAX);
  } else
    zoom_to(zoom_factor * ZOOM_MULTIPLIER);
}

void ImageView::zoom_out(bool step) {
  if (step) {
    auto iter = std::find_if(ZOOM_STEPS.rbegin(), ZOOM_STEPS.rend(),
        [this](double x) { return x < zoom_factor; });
    zoom_to(iter != ZOOM_STEPS.rend() ? *iter : ZOOM_MIN);
  } else
    zoom_to(zoom_factor / ZOOM_MULTIPLIER);
}

void ImageView::show_image() {
  if (!pixbuf)
    return;

  if (zoom_fit == ZOOM_FIT_BEST)
    zoom_factor = scale_to_fit(get_allocated_width(), get_allocated_height(),
        pixbuf->get_width(), pixbuf->get_height(), zoom_fit_expand);
  else if (zoom_fit == ZOOM_FIT_WIDTH) {
    // The function takes two references, but we only need the first value.
    int scrollbar_width = 0, _;
    get_vscrollbar()->get_preferred_width(scrollbar_width, _);
    zoom_factor = scale_to_fit(get_allocated_width(), get_allocated_height(),
        pixbuf->get_width(), pixbuf->get_height(), zoom_fit_expand,
        scrollbar_width);
  }
  zoom_factor = std::max(ZOOM_MIN, std::min(ZOOM_MAX, zoom_factor));
  if (zoom_factor == prev_zoom_factor)
    return;  // Zoom factor didn't change, so there's no need to re-scale.

  image->set(zoom_factor == 1.0 ? pixbuf : pixbuf->scale_simple(
        std::round(pixbuf->get_width() * zoom_factor),
        std::round(pixbuf->get_height() * zoom_factor), Gdk::INTERP_BILINEAR));
  prev_zoom_factor = zoom_factor;
}
