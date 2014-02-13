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

#include "application.h"

#include <giomm/menu.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <gtkmm/builder.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/settings.h>

Application::~Application() {
  delete main_window;
}

// static
Glib::RefPtr<Application> Application::create() {
  return Glib::RefPtr<Application>(new Application());
}

Application::Application() : Gtk::Application("net.beyondboredom.Lumee",
    Gio::APPLICATION_HANDLES_OPEN | Gio::APPLICATION_HANDLES_COMMAND_LINE) {}

void Application::on_startup() {
  Gtk::Application::on_startup();

  add_action("about", sigc::mem_fun(*this, &Application::show_about_dialog));
  add_action("quit", sigc::mem_fun(*this, &Application::hide_all_windows));

  Gtk::Window::set_default_icon_name("emblem-photos");
  Glib::RefPtr<Gtk::Settings> settings = Gtk::Settings::get_default();
  settings->set_property("gtk-application-prefer-dark-theme", true);

  // Load CSS.
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  css_provider->load_from_path(Glib::build_filename(DATA_DIR, "main.css"));
  Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(),
      css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  // Load app menu and main window.
  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();
  builder->add_from_file(Glib::build_filename(DATA_DIR, "app_menu.ui"));
  set_app_menu(Glib::RefPtr<Gio::Menu>::cast_static(builder->get_object(
          "app-menu")));
  builder->add_from_file(Glib::build_filename(DATA_DIR, "main.ui"));
  builder->get_widget_derived("main-window", main_window);
  add_window(*main_window);
}

int Application::on_command_line(
    const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) {
  int argc;
  char** argv = command_line->get_arguments(argc);

  if (argc > 1)
    open(command_line->create_file_for_arg(argv[1]));
  else if (!command_line->is_remote())
    open(Gio::File::create_for_path(Glib::get_user_special_dir(
            G_USER_DIRECTORY_PICTURES)));

  main_window->present();
  g_strfreev(argv);
  return EXIT_SUCCESS;
}

void Application::on_open(const Gio::Application::type_vec_files& files,
    const Glib::ustring& hint) {
  Gtk::Application::on_open(files, hint);
  main_window->open(files[0]);
}

// Keeps only one instance of the dialog and constructs/destructs it as needed.
void Application::show_about_dialog() {
  if (!about_dialog) {
    about_dialog = std::unique_ptr<Gtk::AboutDialog>(new Gtk::AboutDialog());
    about_dialog->set_title(_("About Lumee"));
    about_dialog->set_logo_icon_name("emblem-photos");
    about_dialog->set_program_name("Lumee");
    about_dialog->set_comments(_("A folder-based image viewer."));
    about_dialog->set_copyright("Copyright © 2014 Brian Marshall");
    about_dialog->set_license_type(Gtk::LICENSE_GPL_3_0);
    about_dialog->set_modal();
    about_dialog->signal_response().connect([this](int)
        { about_dialog.reset(); });  // Reset pointer to destroy the dialog.
  }
  about_dialog->set_transient_for(*get_active_window());
  about_dialog->present();
}

void Application::hide_all_windows() {
  for (Gtk::Window* window : get_windows())
    window->hide();
}