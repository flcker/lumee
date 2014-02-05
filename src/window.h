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

#ifndef LUMEE_WINDOW_H
#define LUMEE_WINDOW_H

#include "image_list.h"
#include "image_worker.h"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/image.h>
#include <gtkmm/treeview.h>

class MainWindow : public Gtk::ApplicationWindow {
 public:
  MainWindow(BaseObjectType* cobject,
      const Glib::RefPtr<Gtk::Builder>& builder);
  void open(const Glib::RefPtr<Gio::File>& file);

 private:
  void on_selection_changed();
  void on_image_loaded(const std::shared_ptr<ImageTask>& task);
  void open_file_chooser();

  Gtk::HeaderBar* header_bar;
  Gtk::TreeView* list_view;
  Gtk::Image* image;

  Glib::RefPtr<ImageList> image_list = ImageList::create();
  std::string folder_path;

  ImageWorker image_worker;
  Glib::RefPtr<Gio::Cancellable> image_cancellable;
};

#endif
