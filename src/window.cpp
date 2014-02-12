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

#include "window.h"

#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <gtkmm/filechooserdialog.h>

MainWindow::MainWindow(BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(cobject) {
  builder->get_widget("header-bar", header_bar);
  builder->get_widget("list-view", list_view);
  builder->get_widget("image", image);
  builder->get_widget("image-scroll", image_scroll);

  add_action("open", sigc::mem_fun(*this, &MainWindow::open_file_chooser));

  list_view->set_model(image_list);
  list_view->append_column("", image_list->columns.thumbnail);
  list_view->set_tooltip_column(image_list->columns.escaped_name.index());
  list_view->get_selection()->signal_changed().connect(sigc::mem_fun(*this,
        &MainWindow::on_selection_changed));

  image_worker.signal_finished.connect(sigc::mem_fun(*this,
        &MainWindow::on_image_loaded));
  show_all_children();
}

/**
 * Open a folder.
 */
void MainWindow::open(const Glib::RefPtr<Gio::File>& file) {
  image_list->open_folder(file);
  folder_path = file->get_path();
  header_bar->set_title(Glib::filename_display_basename(folder_path));
}

/**
 * Load an image based on the file list's selection.
 */
void MainWindow::on_selection_changed() {
  image_worker.cancel_all();  // Only one image should be loading at a time
  Gtk::TreeIter iter = list_view->get_selection()->get_selected();
  if (!iter) {  // No selection
    image->clear();
    header_bar->set_subtitle("");
    return;
  }
  image_worker.load((*iter)[image_list->columns.path]);
}

/**
 * Display an image that finished loading.
 */
void MainWindow::on_image_loaded(
    const std::shared_ptr<ImageWorker::Task>& task) {
  image->set(task->pixbuf);
  header_bar->set_subtitle(Glib::filename_display_basename(task->path));
  image_scroll->get_hadjustment()->set_value(0);
  image_scroll->get_vadjustment()->set_value(0);
}

/**
 * Open a folder with a file chooser dialog.
 */
void MainWindow::open_file_chooser() {
  Gtk::FileChooserDialog chooser(*this, _("Open Folder"),
      Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  chooser.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  chooser.add_button(_("_Open"), Gtk::RESPONSE_ACCEPT);
  chooser.set_current_folder(folder_path);
  if (chooser.run() == Gtk::RESPONSE_ACCEPT)
    open(Gio::File::create_for_path(chooser.get_filename()));
}
