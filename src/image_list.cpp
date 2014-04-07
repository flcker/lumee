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

#include "image_list.h"
#include "utils.h"

#include <giomm/file.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
#include <glibmm/miscutils.h>

const int ImageList::THUMBNAIL_SIZE = 96;
const int ImageList::ASYNC_NUM_FILES = 100;
const std::string ImageList::FILE_ATTRIBUTES =
    G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
    G_FILE_ATTRIBUTE_STANDARD_NAME ","
    G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
    G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
    G_FILE_ATTRIBUTE_TIME_MODIFIED;

ImageList::ImageList() {
  set_column_types(columns);
  set_sort_func(columns.display_name_collation_key,
                sigc::mem_fun(*this, &ImageList::compare_display_names));

  // Build a list of supported image MIME types.
  for (Gdk::PixbufFormat format : Gdk::Pixbuf::get_formats()) {
    std::vector<Glib::ustring> mime_types = format.get_mime_types();
    supported_mime_types.insert(end(supported_mime_types),
                                begin(mime_types), end(mime_types));
  }
}

void ImageList::open_folder(const SlotFolderReady& slot,
                            const Glib::RefPtr<Gio::File>& folder) {
  // Cancel everything from the previous folder.
  if (cancellable)
    cancellable->cancel();
  image_worker.cancel_all();
  clear();

  AsyncFolderData data(slot, folder);
  cancellable = data.cancellable;
  folder->enumerate_children_async(
      sigc::bind(sigc::mem_fun(*this, &ImageList::on_enumerate_children),
                 data), cancellable, FILE_ATTRIBUTES);
}

Gtk::TreeModel::iterator ImageList::find(const std::string& path) {
  for (iterator iter : children()) {
    if (std::string((*iter)[columns.path]) == path)
      return iter;
  }
  return iterator();
}

// static
Glib::RefPtr<ImageList> ImageList::create() {
  return Glib::RefPtr<ImageList>(new ImageList());
}

// Gets the enumerator and starts the file loop.
void ImageList::on_enumerate_children(
    const Glib::RefPtr<Gio::AsyncResult>& result, AsyncFolderData& data) {
  try {
    data.enumerator = data.folder->enumerate_children_finish(result);
  } catch (const Gio::Error& error) {
    if (error.code() != Gio::Error::CANCELLED)
      data.slot_folder_ready(false);
    return;
  }
  data.enumerator->next_files_async(
      sigc::bind(sigc::mem_fun(*this, &ImageList::on_next_files), data),
      data.cancellable, ASYNC_NUM_FILES, Glib::PRIORITY_HIGH_IDLE);
}

// Appends all the images in this chunk of files.
void ImageList::on_next_files(const Glib::RefPtr<Gio::AsyncResult>& result,
                              const AsyncFolderData& data) {
  std::vector<Glib::RefPtr<Gio::FileInfo>> files;
  try {
    files = data.enumerator->next_files_finish(result);
  } catch (const Gio::Error& error) {
    if (error.code() != Gio::Error::CANCELLED)
      data.slot_folder_ready(false);
    return;
  }
  for (Glib::RefPtr<Gio::FileInfo> info : files) {
    if (!info->is_hidden() && is_supported_mime_type(info->get_content_type()))
      append_file(data.folder->get_path(), info);
  }

  if (files.size())  // Recurse until there are no more files.
    data.enumerator->next_files_async(
        sigc::bind(sigc::mem_fun(*this, &ImageList::on_next_files), data),
        data.cancellable, ASYNC_NUM_FILES, Glib::PRIORITY_HIGH_IDLE);
  else
    data.slot_folder_ready(true);
}

void ImageList::append_file(const std::string& folder_path,
                            const Glib::RefPtr<Gio::FileInfo>& info) {
  iterator iter = append();
  Row row = *iter;
  row[columns.path] = Glib::build_filename(folder_path, info->get_name());
  row[columns.time_modified] = info->get_attribute_uint64(
      G_FILE_ATTRIBUTE_TIME_MODIFIED);
  row[columns.display_name_collation_key] = collate_key_for_filename(
      info->get_display_name());
  row[columns.tooltip] = "<b>" +
      Glib::Markup::escape_text(info->get_display_name()) + "</b>\n" +
      Glib::Markup::escape_text(Glib::DateTime::create_now_local(
          row[columns.time_modified]).format("%c"));

  row[columns.thumbnail_failed] = false;
  image_worker.load(std::bind(&ImageList::on_thumbnail_loaded, this,
                              std::placeholders::_1, iter),
                    row[columns.path], THUMBNAIL_SIZE);
}

bool ImageList::is_supported_mime_type(const Glib::ustring& mime_type) {
  return std::find(begin(supported_mime_types), end(supported_mime_types),
                   mime_type) != end(supported_mime_types);
}

void ImageList::on_thumbnail_loaded(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
                                    const iterator& iter) {
  if (!iter)  // The file may have been removed from the list by this point.
    return;
  else if (pixbuf)
    (*iter)[columns.thumbnail] = pixbuf;
  else
    (*iter)[columns.thumbnail_failed] = true;
}

int ImageList::compare_display_names(const iterator& iter_a,
                                     const iterator& iter_b) {
  std::string a = (*iter_a)[columns.display_name_collation_key],
              b = (*iter_b)[columns.display_name_collation_key];
  return a.compare(b);
}
