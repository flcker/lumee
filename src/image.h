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

#ifndef LUMEE_IMAGE_H
#define LUMEE_IMAGE_H

#include <gdkmm/pixbuf.h>
#include <gtkmm/treeiter.h>
#include <glibmm/dispatcher.h>
#include <glibmm/threadpool.h>
#include <glibmm/threads.h>

#include <queue>

/**
 * Task that can be processed by ImageWorker.
 */
struct ImageTask {
  const std::string filename;
  const int width_and_height;
  // Unused by ImageWorker, but useful for the thumbnail callback. Maybe
  // there's a cleaner way to store this.
  Gtk::TreeIter iter;

  Glib::RefPtr<Gio::Cancellable> cancellable = Gio::Cancellable::create();
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;

  ImageTask(const std::string& filename, const int width_and_height = 0,
      Gtk::TreeIter iter = Gtk::TreeIter())
      : filename(filename), width_and_height(width_and_height), iter(iter) {}
};

/**
 * Load images in a worker thread and return the results asynchronously.
 *
 * Connect to signal_finished to get notified when an image is done.
 */
class ImageWorker {
  public:
    ImageWorker();
    void load(const std::shared_ptr<ImageTask>& task);

    sigc::signal<void, std::shared_ptr<ImageTask>> signal_finished;

  private:
    void create_pixbuf(const std::shared_ptr<ImageTask>& task);
    void emit_finished();

    Glib::ThreadPool pool;
    Glib::Threads::Mutex mutex;
    std::queue<std::shared_ptr<ImageTask>> queue;
    Glib::Dispatcher dispatcher;
};

#endif
