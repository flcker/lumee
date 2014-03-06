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

#include "image_list.h"

#include <giomm/file.h>
#include <giomm/fileenumerator.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
#include <glibmm/miscutils.h>

const int ImageList::THUMBNAIL_SIZE = 96;

ImageList::ImageList() {
  // Build a list of supported image MIME types.
  for (Gdk::PixbufFormat format : Gdk::Pixbuf::get_formats()) {
    std::vector<Glib::ustring> mime_types = format.get_mime_types();
    supported_mime_types.insert(end(supported_mime_types), begin(mime_types),
        end(mime_types));
  }

  image_worker.signal_finished.connect(sigc::mem_fun(*this,
        &ImageList::on_thumbnail_loaded));
}

// TODO: Make this async.
void ImageList::open_folder(const Glib::RefPtr<Gio::File>& folder) {
  Glib::RefPtr<Gio::FileEnumerator> enumerator = folder->enumerate_children(
      G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
      G_FILE_ATTRIBUTE_STANDARD_NAME ","
      G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
      G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
  image_worker.cancel_all();  // Cancel everything from the previous folder.
  clear();
  while (Glib::RefPtr<Gio::FileInfo> info = enumerator->next_file()) {
    if (info->is_hidden() || !is_supported_mime_type(info->get_content_type()))
      continue;
    Gtk::TreeIter iter = append();
    Gtk::TreeRow row = *iter;
    row[columns.path] = Glib::build_filename(folder->get_path(),
        info->get_name());
    row[columns.escaped_name] = Glib::Markup::escape_text(
        info->get_display_name());
    row[columns.thumbnail] = thumbnail_loading_icon;
    image_worker.load(row[columns.path], THUMBNAIL_SIZE, iter);
  }
}

// static
Glib::RefPtr<ImageList> ImageList::create() {
  Glib::RefPtr<ImageList> model(new ImageList());
  model->set_column_types(model->columns);
  return model;
}

bool ImageList::is_supported_mime_type(const Glib::ustring& mime_type) {
  return std::find(begin(supported_mime_types), end(supported_mime_types),
      mime_type) != end(supported_mime_types);
}

void ImageList::on_thumbnail_loaded(
    const std::shared_ptr<ImageWorker::Task>& task) {
  if (task->iter) {
    Gtk::TreeRow row = *task->iter;
    row[columns.thumbnail] = task->pixbuf;
  }
}
