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

#ifndef LUMEE_MODEL_H
#define LUMEE_MODEL_H

#include "image_worker.h"

#include <gtkmm/icontheme.h>
#include <gtkmm/liststore.h>

class DirectoryModel : public Gtk::ListStore {
 public:
  struct Columns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<std::string> path;
    Gtk::TreeModelColumn<Glib::ustring> escaped_name;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> thumbnail;

    Columns() { add(path); add(escaped_name); add(thumbnail); }
  };

  static const int THUMBNAIL_SIZE;

  DirectoryModel();
  void open(const Glib::RefPtr<Gio::File>& file);
  static Glib::RefPtr<DirectoryModel> create();

  sigc::signal<void, std::string const> signal_path_changed;
  const Columns columns;

 private:
  void on_thumbnail_loaded(const std::shared_ptr<ImageTask>& task);

  ImageWorker image_worker;
  // TODO: Listen for icon theme changes
  const Glib::RefPtr<Gdk::Pixbuf> thumbnail_loading_icon =
      Gtk::IconTheme::get_default()->load_icon("image-loading", 48)->copy();
};

#endif
