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

#ifndef LUMEE_APP_H
#define LUMEE_APP_H

#include "window.h"

#include <gtkmm/application.h>

class LumeeApp : public Gtk::Application {
  public:
    virtual ~LumeeApp();
    static Glib::RefPtr<LumeeApp> create();

  protected:
    LumeeApp();
    virtual void on_startup();
    virtual int on_command_line(
        const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line);
    virtual void on_open(const Gio::Application::type_vec_files& files,
        const Glib::ustring& hint);

  private:
    MainWindow *main_window;
};

#endif
