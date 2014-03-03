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

#ifndef LUMEE_MAIN_WINDOW_H
#define LUMEE_MAIN_WINDOW_H

#include "image_list.h"
#include "image_view.h"
#include "image_worker.h"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/treeview.h>

class MainWindow : public Gtk::ApplicationWindow {
 public:
  MainWindow(BaseObjectType* cobject,
      const Glib::RefPtr<Gtk::Builder>& builder);

  // Opens a folder.
  void open(const Glib::RefPtr<Gio::File>& file);

 private:
  // Opens a folder with a file chooser dialog.
  void open_file_chooser();

  // Loads an image based on the file list's selection.
  void on_selection_changed();

  // Shows an image that has finished loading.
  void on_image_loaded(const std::shared_ptr<ImageWorker::Task>& task);

  // Modifies the image's zoom.
  void on_zoom(const Glib::ustring& mode);
  void on_zoom_to_fit_expand();

  // Enables or disables zooming. Zooming should be disabled if no image is
  // being shown.
  void enable_zoom(bool enabled = true);

  Gtk::HeaderBar* header_bar = nullptr;
  Gtk::Label* zoom_label = nullptr;
  Gtk::TreeView* list_view = nullptr;
  ImageView* image_view = nullptr;

  Glib::RefPtr<Gio::SimpleAction> action_zoom;
  Glib::RefPtr<Gio::SimpleAction> action_zoom_to_fit_expand;

  Glib::RefPtr<ImageList> image_list = ImageList::create();
  std::string folder_path;
  ImageWorker image_worker;
};

#endif  // LUMEE_MAIN_WINDOW_H
