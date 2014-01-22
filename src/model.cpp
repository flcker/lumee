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

#include "model.h"

#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <giomm/file.h>
#include <giomm/fileenumerator.h>

/**
 * Open a directory, clearing the current list of files (if any) and adding
 * image files from the new directory to the list.
 */
void DirectoryModel::open(const Glib::RefPtr<Gio::File>& file) {
  // FIXME: Make this async
  Glib::RefPtr<Gio::FileEnumerator> enumerator = file->enumerate_children(
      G_FILE_ATTRIBUTE_STANDARD_NAME);
  clear();

  while (Glib::RefPtr<Gio::FileInfo> info = enumerator->next_file()) {
    Glib::RefPtr<Gdk::Pixbuf> thumbnail;
    try {
      // FIXME: Don't block the UI
      thumbnail = Gdk::Pixbuf::create_from_file(Glib::build_filename(
            file->get_path(), info->get_name()),
          THUMBNAIL_SIZE, THUMBNAIL_SIZE);
    } catch (const Glib::FileError&) {
      continue;
    } catch (const Gdk::PixbufError&) {
      continue;
    }
    Gtk::TreeRow row = *(append());
    row[columns.filename] = info->get_name();
    row[columns.thumbnail] = thumbnail;
  }
}

Glib::RefPtr<DirectoryModel> DirectoryModel::create() {
  Glib::RefPtr<DirectoryModel> model(new DirectoryModel());
  model->set_column_types(model->columns);
  return model;
}
