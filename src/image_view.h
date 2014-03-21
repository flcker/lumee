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

#ifndef LUMEE_IMAGE_VIEW_H
#define LUMEE_IMAGE_VIEW_H

#include <gtkmm/builder.h>
#include <gtkmm/image.h>
#include <gtkmm/scrolledwindow.h>

// Displays a single image at various zoom settings.
//
// TODO: Improve the zoom performance at large image resolutions.
class ImageView : public Gtk::ScrolledWindow {
 public:
  enum ZoomFit {
    ZOOM_FIT_BEST,   // Fits both width and height.
    ZOOM_FIT_WIDTH,  // Fits only width.
    ZOOM_FIT_NONE    // No automatic fit.
  };

  ImageView(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder);

  // Sets or clears the image.
  void set(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);
  void clear();

  // Returns true if no image is displayed.
  bool empty() const { return !bool(pixbuf); }

  // Returns the current zoom factor.
  double get_zoom() const { return zoom_factor; }

  // Returns true if the zoom factor is currently at the maximum or minimum,
  // respectively.
  bool zoom_is_max() const { return zoom_factor >= ZOOM_MAX; }
  bool zoom_is_min() const { return zoom_factor <= ZOOM_MIN; }

  // Returns the current zoom-to-fit mode.
  ZoomFit get_zoom_fit() const { return zoom_fit; }

  // Zooms in or out. When `step` is true, the zoom factor snaps to the next
  // preset value. Otherwise, a multiplier is used.
  void zoom_in(bool step);
  void zoom_out(bool step);

  // Zooms to the specified factor.
  void zoom_to(double factor);

  // Changes the current zoom-to-fit mode.
  void zoom_to_fit(ZoomFit fit);

  // Expands images to fit. If true, images smaller than the allocated area
  // will be zoomed in when zoom-to-fit is enabled.
  void zoom_to_fit_expand(bool expand);

  // Emitted when the zoom state changes, or an image is set or cleared.
  sigc::signal<void> signal_zoom_changed;

 protected:
  // Keeps the image zoomed to fit (if applicable) when the allocated area
  // changes.
  virtual void on_size_allocate(Gtk::Allocation& allocation);

 private:
  // List of preset zoom factors.
  static const std::vector<double> ZOOM_STEPS;

  // Multiplier for the zoom factor when not using zoom steps.
  static const double ZOOM_MULTIPLIER;

  // Minimum and maximum zoom factors.
  static const double ZOOM_MIN;
  static const double ZOOM_MAX;

  // Updates the image based on current zoom settings.
  void update() { update(get_allocation()); }
  void update(const Gtk::Allocation& allocation);

  Glib::RefPtr<Gdk::Pixbuf> pixbuf;  // Original unscaled pixbuf.
  Gtk::Image image;

  double zoom_factor = 1.0;
  double prev_zoom_factor = 0.0;  // Helps avoid re-scaling at the same factor.
  ZoomFit zoom_fit = ZOOM_FIT_BEST;
  bool zoom_fit_expand = false;
};

#endif
