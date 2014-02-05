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

MainWindow::MainWindow(BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(cobject), model(DirectoryModel::create()) {
  builder->get_widget("header-bar", header_bar);
  builder->get_widget("file-list", file_list);
  builder->get_widget("image", image);

  model->signal_path_changed.connect([this](const std::string& dir_path) {
    header_bar->set_title(Glib::filename_display_basename(dir_path));
  });

  file_list->set_model(model);
  file_list->append_column("", model->columns.thumbnail);
  file_list->set_tooltip_column(model->columns.escaped_name.index());
  file_list->get_selection()->signal_changed().connect(sigc::mem_fun(*this,
        &MainWindow::on_selection_changed));

  image_worker.signal_finished.connect(sigc::mem_fun(*this,
        &MainWindow::on_image_loaded));
  show_all_children();
}

/**
 * Load an image based on the file list's selection.
 */
void MainWindow::on_selection_changed() {
  if (image_cancellable)
    image_cancellable->cancel();  // Only one image should be loading at a time
  Gtk::TreeIter iter = file_list->get_selection()->get_selected();
  if (!iter) {  // No selection
    image->clear();
    header_bar->set_subtitle("");
    return;
  }

  std::shared_ptr<ImageTask> task = std::make_shared<ImageTask>((*iter)[
      model->columns.path]);
  image_cancellable = task->cancellable;
  image_worker.load(task);
}

/**
 * Display an image that finished loading.
 */
void MainWindow::on_image_loaded(const std::shared_ptr<ImageTask>& task) {
  image->set(task->pixbuf);
  header_bar->set_subtitle(Glib::filename_display_basename(task->path));
}
