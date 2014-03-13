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
  add_actions();

  settings->signal_changed().connect(
      sigc::mem_fun(*this, &MainWindow::on_setting_changed));
  list_view->set_model(image_list);
  list_view->append_column("", image_list->columns.thumbnail);
  list_view->set_tooltip_column(image_list->columns.tooltip.index());
  list_view->get_selection()->signal_changed().connect(
      sigc::mem_fun(*this, &MainWindow::on_selection_changed));
  image_view->signal_zoom_changed.connect(
      sigc::mem_fun(*this, &MainWindow::on_zoom_changed));
  image_worker.signal_finished.connect(
      sigc::mem_fun(*this, &MainWindow::on_image_loaded));
  // Call some signal handlers now to initialize them.
  on_zoom_changed();
  on_setting_changed("sort-by");
  on_setting_changed("zoom-to-fit-expand");

  if (settings->get_boolean("maximized"))
    maximize();
  show_all_children();
}

void MainWindow::open(const Glib::RefPtr<Gio::File>& folder) {
  image_list->open_folder(folder);
  folder_path = folder->get_path();
  header_bar->set_title(Glib::filename_display_basename(folder_path));
}

bool MainWindow::on_window_state_event(GdkEventWindowState* event) {
  if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
    settings->set_boolean("maximized",
        event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED);
  return Gtk::ApplicationWindow::on_window_state_event(event);
}

void MainWindow::add_actions() {
  add_action("open", sigc::mem_fun(*this, &MainWindow::open_file_chooser));
  action_zoom_in = add_action("zoom-in",
      sigc::bind(sigc::mem_fun(*this, &MainWindow::zoom_in), true));
  action_zoom_in_no_step = add_action("zoom-in-no-step",
      sigc::bind(sigc::mem_fun(*this, &MainWindow::zoom_in), false));
  action_zoom_out = add_action("zoom-out",
      sigc::bind(sigc::mem_fun(*this, &MainWindow::zoom_out), true));
  action_zoom_out_no_step = add_action("zoom-out-no-step",
      sigc::bind(sigc::mem_fun(*this, &MainWindow::zoom_out), false));
  add_action("zoom-normal", sigc::mem_fun(*this, &MainWindow::zoom_normal));
  action_zoom_to_fit = add_action_radio_string("zoom-to-fit",
      sigc::mem_fun(*this, &MainWindow::zoom_to_fit), "fit-best");
  action_zoom_to_fit_expand = settings->create_action("zoom-to-fit-expand");
  add_action(settings->create_action("sort-by"));
  add_action(settings->create_action("sort-reversed"));
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
  }
}

void MainWindow::on_image_loaded(const ImageWorker::Task& task) {
  image_view->set(task.pixbuf);
  header_bar->set_subtitle(Glib::filename_display_basename(task.path));
  // Reset scroll position.
  image_view->get_hadjustment()->set_value(0);
  image_view->get_vadjustment()->set_value(0);
}

void MainWindow::on_setting_changed(const Glib::ustring& key) {
  if (key == "sort-by" || key == "sort-reversed")
    sort(settings->get_string("sort-by"),
        settings->get_boolean("sort-reversed"));
  else if (key == "zoom-to-fit-expand")
    image_view->zoom_to_fit_expand(settings->get_boolean(key));
}

void MainWindow::zoom_in(bool step) { image_view->zoom_in(step); }
void MainWindow::zoom_out(bool step) { image_view->zoom_out(step); }
void MainWindow::zoom_normal() { image_view->zoom_to(1.0); }

void MainWindow::zoom_to_fit(const Glib::ustring& fit) {
  action_zoom_to_fit->change_state(fit);
  if (fit == "fit-best")
    image_view->zoom_to_fit(ImageView::ZOOM_FIT_BEST);
  else if (fit == "fit-width")
    image_view->zoom_to_fit(ImageView::ZOOM_FIT_WIDTH);
}

void MainWindow::on_zoom_changed() {
  bool can_zoom = !image_view->empty();
  zoom_label->set_text(to_percentage(can_zoom ? image_view->get_zoom() : 1.0));
  zoom_label->get_parent()->get_parent()->set_sensitive(can_zoom);

  action_zoom_in->set_enabled(can_zoom && !image_view->zoom_is_max());
  action_zoom_in_no_step->set_enabled(can_zoom && !image_view->zoom_is_max());
  action_zoom_out->set_enabled(can_zoom && !image_view->zoom_is_min());
  action_zoom_out_no_step->set_enabled(can_zoom && !image_view->zoom_is_min());

  action_zoom_to_fit->set_enabled(can_zoom);
  if (image_view->get_zoom_fit() == ImageView::ZOOM_FIT_NONE) {
    action_zoom_to_fit->change_state(Glib::ustring());  // Clear radio state.
    remove_action("zoom-to-fit-expand");
  } else
    add_action(action_zoom_to_fit_expand);
}

void MainWindow::sort(const Glib::ustring& mode, bool reversed) {
  Gtk::SortType order = reversed ? Gtk::SORT_DESCENDING : Gtk::SORT_ASCENDING;
  if (mode == "name")
    image_list->set_sort_column(image_list->columns.display_name, order);
  else if (mode == "modification-date")
    image_list->set_sort_column(image_list->columns.time_modified, order);
}
