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

#include "application.h"

#include <giomm/menu.h>
#include <glibmm/i18n.h>
#include <glibmm/optioncontext.h>
#include <glibmm/miscutils.h>
#include <gtkmm/builder.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/settings.h>

#include <iostream>

Application::~Application() {
  delete main_window;
}

// static
Glib::RefPtr<Application> Application::create() {
  return Glib::RefPtr<Application>(new Application());
}

Application::Application()
    : Gtk::Application("com.github.bmars.Lumee",
                       Gio::APPLICATION_HANDLES_OPEN |
                       Gio::APPLICATION_HANDLES_COMMAND_LINE) {}

// Returns true to exit without starting the application, false to continue.
bool Application::local_command_line_vfunc(char**& argv, int& exit_status) {
  Glib::OptionContext context("[FOLDER | FILE]");
  Glib::OptionGroup group("", "");
  Glib::OptionGroup group_gtk(gtk_get_option_group(true));
  context.set_main_group(group);
  context.add_group(group_gtk);

  bool show_version = false;
  Glib::OptionEntry entry_version;
  entry_version.set_long_name("version");
  entry_version.set_description(_("Show the version number"));
  group.add_entry(entry_version, show_version);

  int argc = g_strv_length(argv);
  try {
    context.parse(argc, argv);
  } catch (const Glib::OptionError& error) {
    std::cerr << argv[0] << ": " << error.what() << std::endl
              << Glib::ustring::compose(_("Try '%1' for more information."),
                                        std::string(argv[0]) + " --help")
              << std::endl;
    exit_status = EXIT_FAILURE;
    return true;
  }
  if (show_version) {
    std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << std::endl;
    return true;
  }
  return false;
}

void Application::on_startup() {
  Gtk::Application::on_startup();
  add_action("about", sigc::mem_fun(*this, &Application::show_about_dialog));
  add_action("quit", sigc::mem_fun(*this, &Application::hide_all_windows));

  Gtk::Window::set_default_icon_name("image-x-generic");
  Gtk::Settings::get_default()->
      property_gtk_application_prefer_dark_theme() = true;
  try {
    load_ui();
  } catch (const Glib::Error& error) {
    std::cerr << "Error loading UI: " << error.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  // Some of these accelerators aren't shown in the UI with a menu item.
  // Accelerators that do have a menu item may be duplicated here because they
  // aren't automatically added.
  add_accelerator("<Primary>o", "win.open");
  add_accelerator("<Primary>plus", "win.zoom-in");
  add_accelerator("plus", "win.zoom-in-slight");
  add_accelerator("<Primary>minus", "win.zoom-out");
  add_accelerator("minus", "win.zoom-out-slight");
  add_accelerator("<Primary>0", "win.zoom-normal");
  add_accelerator("f", "win.zoom-to-fit", g_variant_new_string("fit-best"));
  add_accelerator("w", "win.zoom-to-fit", g_variant_new_string("fit-width"));
}

int Application::on_command_line(
    const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) {
  int argc = 0;
  char** argv = command_line->get_arguments(argc);
  if (argc > 1)
    open(command_line->create_file_for_arg(argv[1]));
  else if (!command_line->is_remote())
    open(Gio::File::create_for_path(Glib::get_user_special_dir(
        G_USER_DIRECTORY_PICTURES)));
  g_strfreev(argv);
  main_window->present();
  return EXIT_SUCCESS;
}

void Application::on_open(const type_vec_files& files,
                          const Glib::ustring& /*hint*/) {
  main_window->open(files[0]);
}

void Application::load_ui() {
  std::string data_dir = RuntimeInfo::get_data_dir();
  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();
  builder->add_from_file(Glib::build_filename(data_dir, "app_menu.ui"));
  set_app_menu(Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object(
      "app-menu")));
  builder->add_from_file(Glib::build_filename(data_dir, "main.ui"));
  builder->get_widget_derived("main-window", main_window);
  add_window(*main_window);

  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  css_provider->load_from_path(Glib::build_filename(data_dir, "style.css"));
  Gtk::StyleContext::add_provider_for_screen(
      Gdk::Screen::get_default(), css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

// Keeps only one instance of the dialog and constructs/destructs it as needed.
void Application::show_about_dialog() {
  if (!about_dialog) {
    about_dialog = std::unique_ptr<Gtk::AboutDialog>(new Gtk::AboutDialog());
    about_dialog->set_logo_icon_name("image-x-generic");
    about_dialog->set_program_name(PACKAGE_NAME);
    about_dialog->set_version(PACKAGE_VERSION);
    about_dialog->set_comments(_("A folder-based image viewer."));
    about_dialog->set_website(PACKAGE_URL);
    about_dialog->set_copyright("Copyright © 2014 Brian Marshall");
    about_dialog->set_license_type(Gtk::LICENSE_GPL_3_0);
    about_dialog->set_modal();
    // Destroy the dialog when the close button is clicked.
    about_dialog->signal_response().connect(
        [this](int /*response_id*/) { about_dialog.reset(); });
  }
  about_dialog->set_transient_for(*get_active_window());
  about_dialog->present();
}

void Application::hide_all_windows() {
  for (Gtk::Window* window : get_windows())
    window->hide();
}
