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
  void open(const Glib::RefPtr<Gio::File>& folder);

 private:
  // Creates and adds the window actions.
  void add_actions();

  // Opens a folder with a file chooser dialog.
  void open_file_chooser();

  // Loads an image based on the file list's selection.
  void on_selection_changed();

  // Shows an image that has finished loading.
  void on_image_loaded(const std::shared_ptr<ImageWorker::Task>& task);

  // Handlers for zoom actions.
  void zoom_in(bool step);
  void zoom_out(bool step);
  void zoom_normal();
  void zoom_to_fit(const Glib::ustring& mode);
  void zoom_to_fit_expand();
  void on_zoom_changed();

  // Handlers for sort actions.
  void sort(const Glib::ustring& column);
  void sort_reversed();

  Glib::RefPtr<Gio::SimpleAction> action_zoom_in;
  Glib::RefPtr<Gio::SimpleAction> action_zoom_in_no_step;
  Glib::RefPtr<Gio::SimpleAction> action_zoom_out;
  Glib::RefPtr<Gio::SimpleAction> action_zoom_out_no_step;
  Glib::RefPtr<Gio::SimpleAction> action_zoom_to_fit;
  Glib::RefPtr<Gio::SimpleAction> action_zoom_to_fit_expand;
  Glib::RefPtr<Gio::SimpleAction> action_sort;
  Glib::RefPtr<Gio::SimpleAction> action_sort_reversed;

  Gtk::HeaderBar* header_bar = nullptr;
  Gtk::Label* zoom_label = nullptr;
  Gtk::TreeView* list_view = nullptr;
  ImageView* image_view = nullptr;

  Glib::RefPtr<ImageList> image_list = ImageList::create();
  std::string folder_path;
  ImageWorker image_worker;
};

#endif  // LUMEE_MAIN_WINDOW_H
