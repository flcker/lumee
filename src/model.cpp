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

#include "model.h"

#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
#include <glibmm/miscutils.h>
#include <giomm/file.h>
#include <giomm/fileenumerator.h>

const int DirectoryModel::THUMBNAIL_SIZE = 96;

DirectoryModel::DirectoryModel() : Gtk::ListStore() {
  image_worker.signal_finished.connect(sigc::mem_fun(*this,
        &DirectoryModel::on_thumbnail_loaded));
}

/**
 * Open a directory, clearing the current list of files (if any) and adding
 * image files from the new directory to the list.
 */
void DirectoryModel::open(const Glib::RefPtr<Gio::File>& file) {
  // FIXME: Make this async
  Glib::RefPtr<Gio::FileEnumerator> enumerator = file->enumerate_children(
      G_FILE_ATTRIBUTE_STANDARD_NAME ","
      G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
  // TODO: Need to be able to cancel all image_worker thumbnail tasks from the
  // previous directory
  clear();
  path = file->get_path();

  // TODO: Only add files with a supported image format (check content type)
  while (Glib::RefPtr<Gio::FileInfo> info = enumerator->next_file()) {
    Gtk::TreeIter iter = append();
    Gtk::TreeRow row = *iter;
    row[columns.filename] = info->get_name();
    row[columns.escaped_name] = Glib::Markup::escape_text(
        info->get_display_name());
    row[columns.thumbnail] = thumbnail_loading_icon;

    image_worker.load(std::make_shared<ImageTask>(Glib::build_filename(path,
            info->get_name()), THUMBNAIL_SIZE, iter));
  }
}

void DirectoryModel::on_thumbnail_loaded(
    const std::shared_ptr<ImageTask>& task) {
  if (task->iter) {
    Gtk::TreeRow row = *(task->iter);
    row[columns.thumbnail] = task->pixbuf;
  }
}

Glib::RefPtr<DirectoryModel> DirectoryModel::create() {
  Glib::RefPtr<DirectoryModel> model(new DirectoryModel());
  model->set_column_types(model->columns);
  return model;
}
