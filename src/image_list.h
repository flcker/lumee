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

#ifndef LUMEE_IMAGE_LIST_H
#define LUMEE_IMAGE_LIST_H

#include "image_worker.h"

#include <giomm/fileenumerator.h>
#include <gtkmm/liststore.h>

// Model that stores a list of image files with thumbnails.
class ImageList : public Gtk::ListStore {
 public:
  // Function that will be called when a folder has finished opening or failed
  // to open.
  //
  //     void on_folder_ready(bool success)
  typedef sigc::slot<void, bool> SlotFolderReady;

  struct Columns : public Gtk::TreeModelColumnRecord {
    Columns() { add(path); add(time_modified); add(thumbnail);
                add(display_name_collation_key); add(tooltip);
                add(thumbnail_failed); }

    Gtk::TreeModelColumn<std::string> path;
    Gtk::TreeModelColumn<guint64> time_modified;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> thumbnail;
    Gtk::TreeModelColumn<std::string> display_name_collation_key;
    Gtk::TreeModelColumn<Glib::ustring> tooltip;
    Gtk::TreeModelColumn<bool> thumbnail_failed;
  };

  ImageList();

  // Opens a folder asynchronously, clearing the list and adding images from
  // this folder.
  void open_folder(const SlotFolderReady& slot,
                   const Glib::RefPtr<Gio::File>& folder);

  // Searches for an image with a given file path. If not found, returns an
  // empty iterator.
  iterator find(const std::string& path);

  // Creates a new instance.
  static Glib::RefPtr<ImageList> create();

  const Columns columns;

 private:
  // Data used while asynchronously opening a folder.
  struct AsyncFolderData {
    AsyncFolderData(const SlotFolderReady& slot,
                    const Glib::RefPtr<Gio::File>& folder)
        : slot_folder_ready(slot), folder(folder) {}

    SlotFolderReady slot_folder_ready;
    Glib::RefPtr<Gio::File> folder;
    Glib::RefPtr<Gio::FileEnumerator> enumerator;
    Glib::RefPtr<Gio::Cancellable> cancellable = Gio::Cancellable::create();
  };

  static const int THUMBNAIL_SIZE;
  static const int ASYNC_NUM_FILES;
  static const std::string FILE_ATTRIBUTES;

  // Handlers for asynchronously opening a folder.
  void on_enumerate_children(const Glib::RefPtr<Gio::AsyncResult>& result,
                             AsyncFolderData& data);
  void on_next_files(const Glib::RefPtr<Gio::AsyncResult>& result,
                     const AsyncFolderData& data);

  // Appends an image file to the list. `folder_path` is the folder containing
  // the image, and `info` is the image file itself.
  void append_file(const std::string& folder_path,
                   const Glib::RefPtr<Gio::FileInfo>& info);

  // Returns true if the MIME type is a supported image format.
  bool is_supported_mime_type(const Glib::ustring& mime_type);

  // Updates a row with its thumbnail.
  void on_thumbnail_loaded(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
                           const iterator& iter);

  // Compares the order of two file display names.
  int compare_display_names(const iterator& iter_a, const iterator& iter_b);

  std::vector<Glib::ustring> supported_mime_types;
  ImageWorker image_worker;

  // Cancellable from the most recent `AsyncFolderData`.
  Glib::RefPtr<Gio::Cancellable> cancellable;
};

#endif  // LUMEE_IMAGE_LIST_H
