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

#include <giomm/settings.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/stack.h>
#include <gtkmm/treeview.h>

class MainWindow : public Gtk::ApplicationWindow {
 public:
  MainWindow(BaseObjectType* cobject,
      const Glib::RefPtr<Gtk::Builder>& builder);

  // Opens a folder.
  void open(const Glib::RefPtr<Gio::File>& folder);

 protected:
  // Saves the window's maximized state to a setting.
  virtual bool on_window_state_event(GdkEventWindowState* event);

 private:
  // Creates and adds the window actions.
  void add_actions();

  // Opens a folder with a file chooser dialog.
  void open_file_chooser();

  // Shows thumbnails in the list view.
  void on_thumbnail_cell_data(Gtk::CellRenderer* cell,
      const Gtk::TreeIter& iter);

  // Loads an image based on the file list's selection.
  void on_selection_changed();

  // Shows an image that has finished loading.
  void on_image_loaded(const ImageWorker::Task& task);

  void on_setting_changed(const Glib::ustring& key);

  // Handlers for zoom actions.
  void zoom_in(bool step);
  void zoom_out(bool step);
  void zoom_normal();
  void zoom_to_fit(const Glib::ustring& fit);
  void on_zoom_changed();

  void sort(const Glib::ustring& mode, bool reversed);

  Glib::RefPtr<Gio::Settings> settings = Gio::Settings::create(
      "com.github.bmars.Lumee");
  Glib::RefPtr<Gio::SimpleAction> action_zoom_in, action_zoom_in_no_step,
      action_zoom_out, action_zoom_out_no_step, action_zoom_to_fit;
  Glib::RefPtr<Gio::Action> action_zoom_to_fit_expand;

  Gtk::HeaderBar* header_bar = nullptr;
  Gtk::Label* zoom_label = nullptr;
  Gtk::TreeView* list_view = nullptr;
  Gtk::Stack* stack = nullptr;
  ImageView* image_view = nullptr;
  Gtk::Label* message = nullptr;

  Glib::RefPtr<ImageList> image_list = ImageList::create();
  std::string folder_path;
  ImageWorker image_worker;
};

#endif  // LUMEE_MAIN_WINDOW_H
