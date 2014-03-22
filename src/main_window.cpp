// Copyright (C) 2014 Brian Marshall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
  builder->get_widget("stack", stack);
  builder->get_widget_derived("image-view", image_view);
  builder->get_widget("message-icon", message_icon);
  builder->get_widget("message", message);
  add_actions();

  list_view->set_model(image_list);
  list_view->set_tooltip_column(image_list->columns.tooltip.index());
  list_view->get_column(0)->set_cell_data_func(
      *list_view->get_column_cell_renderer(0),
      sigc::mem_fun(*this, &MainWindow::on_thumbnail_cell_data));
  list_view->get_selection()->signal_changed().connect(sigc::mem_fun(
      *this, &MainWindow::on_selection_changed));
  image_view->signal_zoom_changed.connect(sigc::mem_fun(
      *this, &MainWindow::on_zoom_changed));
  settings->signal_changed().connect(sigc::mem_fun(
      *this, &MainWindow::on_setting_changed));

  // Call some signal handlers now to initialize them.
  on_zoom_changed();
  on_setting_changed("sort-by");
  on_setting_changed("zoom-to-fit-expand");
  if (settings->get_boolean("maximized"))
    maximize();
  show_all_children();
}

void MainWindow::open(Glib::RefPtr<Gio::File> file) {
  Glib::RefPtr<Gio::File> file_to_select;
  if (file->query_file_type() == Gio::FILE_TYPE_REGULAR) {
    // Open the folder the file is in, and select it.
    file_to_select = file;
    file = file->get_parent();
  }

  list_view->get_selection()->unselect_all();
  image_list->open_folder(sigc::bind(sigc::mem_fun(
      *this, &MainWindow::on_folder_ready), file_to_select), file);
  folder_path = file->get_path();
  header_bar->set_title(Glib::filename_display_basename(folder_path));
}

bool MainWindow::on_window_state_event(GdkEventWindowState* event) {
  if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
    settings->set_boolean(
        "maximized", event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED);
  return Gtk::ApplicationWindow::on_window_state_event(event);
}

void MainWindow::add_actions() {
  add_action("open", sigc::mem_fun(*this, &MainWindow::open_file_chooser));
  action_zoom_in = add_action(
      "zoom-in",
      sigc::bind(sigc::mem_fun(image_view, &ImageView::zoom_in), true));
  action_zoom_in_no_step = add_action(
      "zoom-in-no-step",
      sigc::bind(sigc::mem_fun(image_view, &ImageView::zoom_in), false));
  action_zoom_out = add_action(
      "zoom-out",
      sigc::bind(sigc::mem_fun(image_view, &ImageView::zoom_out), true));
  action_zoom_out_no_step = add_action(
      "zoom-out-no-step",
      sigc::bind(sigc::mem_fun(image_view, &ImageView::zoom_out), false));
  add_action("zoom-normal",
             sigc::bind(sigc::mem_fun(image_view, &ImageView::zoom_to), 1.0));
  action_zoom_to_fit = add_action_radio_string(
      "zoom-to-fit", sigc::mem_fun(*this, &MainWindow::zoom_to_fit),
      "fit-best");
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

// In addition to showing a thumbnail, thumbnail cells can be in a "failed" or
// "loading" state.
void MainWindow::on_thumbnail_cell_data(Gtk::CellRenderer* cell_base,
                                        const Gtk::TreeModel::iterator& iter) {
  auto cell = dynamic_cast<Gtk::CellRendererPixbuf*>(cell_base);
  Glib::RefPtr<Gdk::Pixbuf> thumbnail = (*iter)[image_list->columns.thumbnail];
  if ((*iter)[image_list->columns.thumbnail_failed])
    cell->property_icon_name() = "image-x-generic";
  else if (!thumbnail)
    cell->property_icon_name() = "image-loading";
  else
    cell->property_pixbuf() = thumbnail;
}

void MainWindow::on_selection_changed() {
  image_worker.cancel_all();  // Only one image should be loading at a time.
  Gtk::TreeModel::iterator iter = list_view->get_selection()->get_selected();
  if (iter) {
    std::string path = (*iter)[image_list->columns.path];
    image_worker.load(std::bind(&MainWindow::on_image_loaded, this,
                                std::placeholders::_1, path), path);
  } else {  // No selection.
    image_view->clear();
    stack->set_visible_child(*image_view);
    header_bar->set_subtitle("");
  }
}

void MainWindow::on_image_loaded(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
                                 const std::string& path) {
  if (pixbuf) {
    image_view->set(pixbuf);
    // Reset scroll position.
    image_view->get_hadjustment()->set_value(0);
    image_view->get_vadjustment()->set_value(0);
    stack->set_visible_child(*image_view);
  } else {
    show_message(_("Could not load this image"));
    image_view->clear();
  }
  header_bar->set_subtitle(Glib::filename_display_basename(path));
}

void MainWindow::on_folder_ready(
    bool success, const Glib::RefPtr<Gio::File>& file_to_select) {
  if (!success)
    show_message(_("Could not open this folder"));
  else if (!image_list->children().size())
    show_message(_("No images in this folder"), "emblem-photos-symbolic");
  else if (!list_view->get_selection()->get_selected()) {
    // Since there is no selection yet, select and scroll to either the first
    // image, or a requested one.
    Gtk::TreeModel::Path path("0");
    if (file_to_select) {
      if (Gtk::TreeModel::iterator iter =
              image_list->find(file_to_select->get_path()))
        path = image_list->get_path(iter);
    }
    list_view->get_selection()->select(path);
    list_view->scroll_to_row(path);
  }
}

void MainWindow::on_setting_changed(const Glib::ustring& key) {
  if (key == "sort-by" || key == "sort-reversed")
    sort(settings->get_string("sort-by"),
         settings->get_boolean("sort-reversed"));
  else if (key == "zoom-to-fit-expand")
    image_view->zoom_to_fit_expand(settings->get_boolean(key));
}

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

void MainWindow::show_message(const Glib::ustring& text,
                              const Glib::ustring& icon_name) {
  message_icon->property_icon_name() = icon_name;
  message->set_text(text);
  stack->set_visible_child("message-area");
}
