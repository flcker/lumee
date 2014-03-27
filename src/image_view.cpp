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

#include "image_view.h"
#include "utils.h"

#include <gtkmm/adjustment.h>
#include <gtkmm/scrollbar.h>

const std::vector<double> ImageView::ZOOM_STEPS =
    {0.10, 0.20, 1/3., 0.5, 2/3., 1.0, 1.5, 2.0, 2.5, 3.0};
const double ImageView::ZOOM_MULTIPLIER = 1.1;
const double ImageView::ZOOM_MIN = ZOOM_STEPS.front();
const double ImageView::ZOOM_MAX = ZOOM_STEPS.back();

ImageView::ImageView(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& /*builder*/)
    : Gtk::ScrolledWindow(cobject) {
  add(image);
  hadjustment->signal_changed().connect(sigc::bind(sigc::mem_fun(
      *this, &ImageView::on_adjustment_changed), Gtk::ORIENTATION_HORIZONTAL));
  vadjustment->signal_changed().connect(sigc::bind(sigc::mem_fun(
      *this, &ImageView::on_adjustment_changed), Gtk::ORIENTATION_VERTICAL));
}

void ImageView::set(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf) {
  this->pixbuf = pixbuf;
  prev_zoom_factor = hadjustment_zoom_factor = vadjustment_zoom_factor = 0.0;
  update();
  anchor = Point(pixbuf->get_width() * 0.5, 0.0);  // Scroll to the top center.
  signal_zoom_changed.emit();
}

void ImageView::clear() {
  pixbuf.reset();
  image.clear();
  signal_zoom_changed.emit();
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

void ImageView::zoom_to(double factor) {
  zoom_factor = factor;
  zoom_to_fit(ZOOM_FIT_NONE);
}

void ImageView::zoom_to_fit(ZoomFit fit) {
  zoom_fit = fit;
  update();
  signal_zoom_changed.emit();
}

void ImageView::zoom_to_fit_expand(bool expand) {
  zoom_fit_expand = expand;
  update();
  signal_zoom_changed.emit();
}

void ImageView::on_size_allocate(Gtk::Allocation& allocation) {
  update(allocation);
  Gtk::ScrolledWindow::on_size_allocate(allocation);
  signal_zoom_changed.emit();
}

void ImageView::update(const Gtk::Allocation& allocation) {
  if (empty())
    return;
  else if (zoom_fit == ZOOM_FIT_BEST)
    zoom_factor = scale_to_fit(allocation.get_width(), allocation.get_height(),
                               pixbuf->get_width(), pixbuf->get_height(),
                               zoom_fit_expand);
  else if (zoom_fit == ZOOM_FIT_WIDTH) {
    int scrollbar_width = 0, _;  // `_` is an unused placeholder.
    get_vscrollbar()->get_preferred_width(scrollbar_width, _);
    zoom_factor = scale_to_fit(allocation.get_width(), allocation.get_height(),
                               pixbuf->get_width(), pixbuf->get_height(),
                               zoom_fit_expand, scrollbar_width);
  }
  zoom_factor = std::max(ZOOM_MIN, std::min(ZOOM_MAX, zoom_factor));

  if (zoom_factor != prev_zoom_factor) {
    anchor = get_center();
    image.set(zoom_factor == 1.0 ? pixbuf : pixbuf->scale_simple(
        std::round(pixbuf->get_width() * zoom_factor),
        std::round(pixbuf->get_height() * zoom_factor), Gdk::INTERP_BILINEAR));
    prev_zoom_factor = zoom_factor;
  }
}

// Defaults to the absolute center of the pixbuf. If the scaled image's width
// or height is larger than the allocated area, calculates the visible center
// based on scroll position and zoom factor.
Point ImageView::get_center() const {
  Point point(pixbuf->get_width() * 0.5, pixbuf->get_height() * 0.5);
  if (hadjustment->get_upper() > hadjustment->get_page_size())
    point.x = (hadjustment->get_value() + hadjustment->get_page_size() * 0.5) /
              hadjustment_zoom_factor;
  if (vadjustment->get_upper() > vadjustment->get_page_size())
    point.y = (vadjustment->get_value() + vadjustment->get_page_size() * 0.5) /
              vadjustment_zoom_factor;
  return point;
}

// Centers the anchor point in response to `update()` zooming the image. The
// horizontal and vertical adjustments are changed at practically the same
// time, but they emit separate signals, so this function handles each one
// separately.
void ImageView::on_adjustment_changed(Gtk::Orientation orientation) {
  if (orientation == Gtk::ORIENTATION_HORIZONTAL &&
      zoom_factor != hadjustment_zoom_factor) {
    hadjustment->set_value(anchor.x * zoom_factor -
                           hadjustment->get_page_size() * 0.5);
    hadjustment_zoom_factor = zoom_factor;
  } else if (orientation == Gtk::ORIENTATION_VERTICAL &&
             zoom_factor != vadjustment_zoom_factor) {
    vadjustment->set_value(anchor.y * zoom_factor -
                           vadjustment->get_page_size() * 0.5);
    vadjustment_zoom_factor = zoom_factor;
  }
}
