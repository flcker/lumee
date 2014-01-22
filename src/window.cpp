/*
 * Copyright (C) 2014 Brian Marshall
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
#include "model.h"

#include <gtkmm/iconview.h>

MainWindow::MainWindow(BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::ApplicationWindow(cobject) {
  Glib::RefPtr<DirectoryModel> model = DirectoryModel::create();

  // Set the IconView's model
  Gtk::IconView *iv;
  builder->get_widget("icon-view", iv);
  iv->set_model(model);
  iv->set_pixbuf_column(model->columns.thumbnail);
  iv->set_tooltip_column(model->columns.filename.index());

  show_all_children();
}
