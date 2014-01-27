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
  builder->get_widget("icon-view", icon_view);
  builder->get_widget("image", image);

  icon_view->set_model(model);
  icon_view->set_pixbuf_column(model->columns.thumbnail);
  icon_view->set_tooltip_column(model->columns.escaped_name.index());
  icon_view->signal_selection_changed().connect(sigc::mem_fun(*this,
        &MainWindow::on_selection_changed));

  image_worker.signal_finished.connect(
      [this](Glib::RefPtr<Gdk::Pixbuf> pixbuf) { image->set(pixbuf); });

  show_all_children();
}

/**
 * Set the image based on the icon view's selection.
 */
void MainWindow::on_selection_changed() {
  std::string filename;
  try {
    filename = (*(model->get_iter(icon_view->get_selected_items().at(0))))[
        model->columns.filename];
  } catch (const std::out_of_range&) {
    image->clear();   // No selection
    return;
  }

  if (image_cancellable)
    image_cancellable->cancel();  // Only one image should be loading at a time
  image_cancellable = Gio::Cancellable::create();
  image_worker.load(Glib::build_filename(model->path, filename),
      image_cancellable);
}
