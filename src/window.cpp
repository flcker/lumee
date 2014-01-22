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

#include <gtkmm/iconview.h>

MainWindow::MainWindow(BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::ApplicationWindow(cobject), model(DirectoryModel::create()) {
  // Set the IconView's model
  Gtk::IconView* icon_view;
  builder->get_widget("icon-view", icon_view);
  icon_view->set_model(model);
  icon_view->set_pixbuf_column(model->columns.thumbnail);
  icon_view->set_tooltip_column(model->columns.filename.index());

  show_all_children();
}
