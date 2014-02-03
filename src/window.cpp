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

#include <glibmm/miscutils.h>

MainWindow::MainWindow(BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(cobject), model(DirectoryModel::create()) {
  builder->get_widget("file-list", file_list);
  builder->get_widget("image", image);

  file_list->set_model(model);
  file_list->append_column("", model->columns.thumbnail);
  file_list->set_tooltip_column(model->columns.escaped_name.index());
  file_list->get_selection()->signal_changed().connect(sigc::mem_fun(*this,
        &MainWindow::on_selection_changed));

  image_worker.signal_finished.connect([this](
        const std::shared_ptr<ImageTask>& task) { image->set(task->pixbuf); });
  show_all_children();
}

/**
 * Set the image based on the file list's selection.
 */
void MainWindow::on_selection_changed() {
  if (image_cancellable)
    image_cancellable->cancel();  // Only one image should be loading at a time
  Gtk::TreeIter iter = file_list->get_selection()->get_selected();
  if (!iter) {  // No selection
    image->clear();
    return;
  }

  std::string filename = (*iter)[model->columns.filename];
  std::shared_ptr<ImageTask> task = std::make_shared<ImageTask>(
      Glib::build_filename(model->path, filename));
  image_cancellable = task->cancellable;
  image_worker.load(task);
}
