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

#include <giomm/fileenumerator.h>
#include <gtkmm/liststore.h>

// A model that stores a list of image files with thumbnails.
class ImageList : public Gtk::ListStore {
 public:
  // Function that will be called when opening a folder has completed,
  // successfully or not.
  //
  // void on_folder_ready(bool success)
  typedef sigc::slot<void, bool> SlotFolderReady;

  struct Columns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<std::string> path;
    Gtk::TreeModelColumn<Glib::ustring> display_name;
    Gtk::TreeModelColumn<guint64> time_modified;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> thumbnail;
    Gtk::TreeModelColumn<bool> thumbnail_failed;
    Gtk::TreeModelColumn<Glib::ustring> tooltip;

    Columns() { add(path); add(display_name); add(time_modified);
      add(thumbnail); add(thumbnail_failed); add(tooltip); }
  };

  ImageList();

  // Opens a folder asynchronously, replacing the current list with the
  // folder's images.
  void open_folder(const SlotFolderReady& slot,
      const Glib::RefPtr<Gio::File>& folder);

  // Searches for an image with a given file path. If not found, returns an
  // empty iterator.
  iterator find(const std::string& path);

  static Glib::RefPtr<ImageList> create();

  const Columns columns;

 private:
  // Data used while asynchronously opening a folder.
  struct AsyncFolderData {
    AsyncFolderData(const SlotFolderReady& slot,
        const Glib::RefPtr<Gio::File>& folder)
        : slot_folder_ready(slot), folder(folder) {}

    SlotFolderReady slot_folder_ready;
    Glib::RefPtr<Gio::Cancellable> cancellable = Gio::Cancellable::create();
    Glib::RefPtr<Gio::File> folder;
    Glib::RefPtr<Gio::FileEnumerator> enumerator;
  };

  static const int THUMBNAIL_SIZE;
  static const int ASYNC_NUM_FILES;
  static const std::string FILE_ATTRIBUTES;

  // Handlers for asynchronously opening a folder.
  void on_enumerate_children(const Glib::RefPtr<Gio::AsyncResult>& result,
      AsyncFolderData& data);
  void on_next_files(const Glib::RefPtr<Gio::AsyncResult>& result,
      const AsyncFolderData& data);

  // Appends an image file to the list.
  void append_image(const std::string& folder_path,
      const Glib::RefPtr<Gio::FileInfo>& info);

  bool is_supported_mime_type(const Glib::ustring& mime_type);
  void on_thumbnail_loaded(const ImageWorker::Task& task);

  std::vector<Glib::ustring> supported_mime_types;
  ImageWorker image_worker;
  Glib::RefPtr<Gio::Cancellable> cancellable;
};

#endif  // LUMEE_IMAGE_LIST_H
