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

#ifndef LUMEE_IMAGE_VIEW_H
#define LUMEE_IMAGE_VIEW_H

#include <gtkmm/builder.h>
#include <gtkmm/image.h>
#include <gtkmm/scrolledwindow.h>

// Displays a single image at various zoom settings.
//
// TODO: Improve the zoom performance at large image resolutions.
// TODO: In best fit mode, re-zoom the image when the window is resized.
class ImageView : public Gtk::ScrolledWindow {
 public:
  enum ZoomFit {
    ZOOM_FIT_BEST,   // Fits both width and height.
    ZOOM_FIT_WIDTH,  // Fits only width.
    ZOOM_FIT_NONE    // No automatic fit.
  };

  // List of preset zoom factors.
  static const std::vector<double> ZOOM_STEPS;

  // Multiplier for the zoom factor when not using zoom steps.
  static const double ZOOM_MULTIPLIER;

  // Minimum and maximum zoom factors.
  static const double ZOOM_MIN;
  static const double ZOOM_MAX;

  ImageView(BaseObjectType* cobject,
      const Glib::RefPtr<Gtk::Builder>& builder);

  void set(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);
  void clear();
  bool empty() const { return !bool(pixbuf); }

  double get_zoom() const { return zoom_factor; }

  void zoom_to_fit(ZoomFit fit) {
    zoom_fit = fit;
    show_image();
    signal_zoom_changed.emit();
  }

  // Expands images to fit. If true, images smaller than the allocated area
  // will be zoomed in when zoom-to-fit is enabled.
  void zoom_to_fit_expand(bool expand) {
    zoom_fit_expand = expand;
    show_image();
  }

  void zoom_to(double factor) {
    zoom_factor = factor;
    zoom_to_fit(ZOOM_FIT_NONE);
  }

  // Zooms in or out. When 'step' is true, the zoom factor snaps to the next
  // preset value.
  void zoom_in(bool step = false);
  void zoom_out(bool step = false);

  // Emitted when the zoom state changes, or an image is set or cleared.
  sigc::signal<void> signal_zoom_changed;

 private:
  // Shows the image based on current zoom settings.
  void show_image();

  // The original unscaled pixbuf.
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;

  ZoomFit zoom_fit = ZOOM_FIT_BEST;
  bool zoom_fit_expand = false;
  double zoom_factor = 1.0;

  // Used to avoid re-scaling the image when the zoom factor stays the same.
  double prev_zoom_factor = 0.0;

  Gtk::Image* image = nullptr;
};

#endif
