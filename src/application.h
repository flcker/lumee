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

#ifndef LUMEE_APPLICATION_H
#define LUMEE_APPLICATION_H

#include "main_window.h"
#include "utils.h"

#include <gtkmm/aboutdialog.h>
#include <gtkmm/application.h>

class Application : public Gtk::Application {
 public:
  virtual ~Application();

  static Glib::RefPtr<Application> create();

 protected:
  Application();

  virtual bool local_command_line_vfunc(char**& argv, int& exit_status);
  virtual void on_startup();
  virtual int on_command_line(
      const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line);
  virtual void on_open(const type_vec_files& files, const Glib::ustring& hint);

 private:
  // Loads the CSS and UI at startup.
  void load_ui();

  void show_about_dialog();
  void hide_all_windows();

  MainWindow* main_window = nullptr;
  std::unique_ptr<Gtk::AboutDialog> about_dialog;
};

#endif  // LUMEE_APPLICATION_H
