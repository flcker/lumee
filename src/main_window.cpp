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

#include "main_window.h"
#include "utils.h"

#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <gtkmm/filechooserdialog.h>

MainWindow::MainWindow(BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(cobject) {
  builder->get_widget("header-bar", header_bar);
  builder->get_widget("zoom-label", zoom_label);
  builder->get_widget("list-view", list_view);
  builder->get_widget_derived("image-view", image_view);

  add_action("open", sigc::mem_fun(*this, &MainWindow::open_file_chooser));
  action_zoom = add_action_radio_string("zoom", sigc::mem_fun(*this,
        &MainWindow::on_zoom), "best-fit");
  action_zoom_to_fit_expand = add_action_bool("zoom-to-fit-expand",
      sigc::mem_fun(*this, &MainWindow::on_zoom_to_fit_expand));
  enable_zoom(false);

  list_view->set_model(image_list);
  list_view->append_column("", image_list->columns.thumbnail);
  list_view->set_tooltip_column(image_list->columns.escaped_name.index());
  list_view->get_selection()->signal_changed().connect(sigc::mem_fun(*this,
        &MainWindow::on_selection_changed));

  image_worker.signal_finished.connect(sigc::mem_fun(*this,
        &MainWindow::on_image_loaded));
  show_all_children();
}

void MainWindow::open(const Glib::RefPtr<Gio::File>& file) {
  image_list->open_folder(file);
  folder_path = file->get_path();
  header_bar->set_title(Glib::filename_display_basename(folder_path));
}

void MainWindow::open_file_chooser() {
  Gtk::FileChooserDialog chooser(*this, _("Open Folder"),
      Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  chooser.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  chooser.add_button(_("_Open"), Gtk::RESPONSE_ACCEPT);
  chooser.set_current_folder(folder_path);
  if (chooser.run() == Gtk::RESPONSE_ACCEPT)
    open(chooser.get_file());
}

void MainWindow::on_selection_changed() {
  image_worker.cancel_all();  // Only one image should be loading at a time.
  Gtk::TreeIter iter = list_view->get_selection()->get_selected();
  if (iter)
    image_worker.load((*iter)[image_list->columns.path]);
  else {  // No selection.
    image_view->clear();
    header_bar->set_subtitle("");
    enable_zoom(false);
  }
}

void MainWindow::on_image_loaded(
    const std::shared_ptr<ImageWorker::Task>& task) {
  image_view->set(task->pixbuf);
  header_bar->set_subtitle(Glib::filename_display_basename(task->path));
  enable_zoom();
  // Reset scroll position.
  image_view->get_hadjustment()->set_value(0);
  image_view->get_vadjustment()->set_value(0);
}

void MainWindow::on_zoom(const Glib::ustring& mode) {
  if (mode == "best-fit" || mode == "fit-width") {
    if (mode == "best-fit")
      image_view->zoom_to_fit(image_view->ZOOM_FIT_BEST);
    else if (mode == "fit-width")
      image_view->zoom_to_fit(image_view->ZOOM_FIT_WIDTH);
    action_zoom->change_state(mode);
    action_zoom_to_fit_expand->set_enabled();
  } else {
    if (mode == "normal")
      image_view->zoom_to(1.0);
    else if (mode == "in" || mode == "in::step")
      image_view->zoom_in(mode == "in::step");
    else if (mode == "out" || mode == "out::step")
      image_view->zoom_out(mode == "out::step");
    action_zoom->change_state(Glib::ustring());
    action_zoom_to_fit_expand->set_enabled(false);
  }
  zoom_label->set_text(to_percentage(image_view->get_zoom()));
}

void MainWindow::on_zoom_to_fit_expand() {
  bool expand = false;
  action_zoom_to_fit_expand->get_state(expand);
  action_zoom_to_fit_expand->change_state(!expand);
  image_view->zoom_to_fit_expand(!expand);
  zoom_label->set_text(to_percentage(image_view->get_zoom()));
}

void MainWindow::enable_zoom(bool enabled) {
  if (enabled) {
    zoom_label->get_parent()->get_parent()->set_sensitive();
    zoom_label->set_text(to_percentage(image_view->get_zoom()));
    action_zoom->set_enabled();
  } else {
    zoom_label->get_parent()->get_parent()->set_sensitive(false);
    zoom_label->set_text("100%");
    action_zoom->set_enabled(false);
  }
}
