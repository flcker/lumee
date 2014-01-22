/*
 * Copyright (C) 2014 Brian Marshall
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

#include <gtkmm/liststore.h>

class DirectoryModel : public Gtk::ListStore {
  public:
    void open(const Glib::RefPtr<Gio::File>&);
    static Glib::RefPtr<DirectoryModel> create();

    struct Columns : public Gtk::TreeModelColumnRecord {
      Gtk::TreeModelColumn<std::string> filename;
      Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > thumbnail;

      Columns() { add(filename); add(thumbnail); }
    };
    const Columns columns;
    static const int THUMBNAIL_SIZE = 96;
};

#endif
