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

#ifndef LUMEE_IMAGE_LIST_H
#define LUMEE_IMAGE_LIST_H

#include "image_worker.h"

#include <gtkmm/icontheme.h>
#include <gtkmm/liststore.h>

/**
 * A model that stores a list of image files with thumbnails.
 */
class ImageList : public Gtk::ListStore {
 public:
  struct Columns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<std::string> path;
    Gtk::TreeModelColumn<Glib::ustring> escaped_name;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> thumbnail;

    Columns() { add(path); add(escaped_name); add(thumbnail); }
  };

  static const int THUMBNAIL_SIZE;

  ImageList();
  void open_folder(const Glib::RefPtr<Gio::File>& file);
  static Glib::RefPtr<ImageList> create();

  const Columns columns;

 private:
  bool is_supported_mime_type(const Glib::ustring& mime_type);
  void on_thumbnail_loaded(const std::shared_ptr<ImageWorker::Task>& task);

  std::vector<Glib::ustring> supported_mime_types;
  ImageWorker image_worker;
  // TODO: Listen for icon theme changes
  const Glib::RefPtr<Gdk::Pixbuf> thumbnail_loading_icon =
      Gtk::IconTheme::get_default()->load_icon("image-loading", 48)->copy();
};

#endif
