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

#ifndef LUMEE_IMAGE_WORKER_H
#define LUMEE_IMAGE_WORKER_H

#include "work_queue.h"

#include <gdkmm/pixbuf.h>
#include <gtkmm/treeiter.h>
#include <glibmm/dispatcher.h>
#include <glibmm/threads.h>

#include <queue>

// Loads images in a thread and returns the results asynchronously.
class ImageWorker {
 public:
  // A task that can be processed.
  struct Task {
    // Path to the image file.
    const std::string path;

    // Size to scale the image to, if any (assumes an equal width and height).
    const int scale_size;

    // Will contain the result of the task.
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    // Unused by ImageWorker, but useful for the thumbnail callback. Maybe
    // there's a cleaner way to store this.
    Gtk::TreeIter iter;

    Task(const std::string& path, const int scale_size, Gtk::TreeIter iter)
        : path(path), scale_size(scale_size), iter(iter) {}
  };

  ImageWorker();
  ~ImageWorker();

  // Adds a loading task to the queue.
  void load(const std::string& path, const int scale_size = 0,
      Gtk::TreeIter iter = Gtk::TreeIter());

  // Cancels the running task and removes all queued tasks.
  void cancel_all();

  // Notifies when an image is done.
  sigc::signal<void, std::shared_ptr<Task>> signal_finished;

 private:
  // Processes a slot and task, communicating the result to the main thread.
  void process(const sigc::slot<void, std::shared_ptr<Task>>& slot,
      const std::shared_ptr<Task>& task);

  // Loads an image.
  void do_load(const std::shared_ptr<Task>& task);

  void emit_finished();

  WorkQueue work_queue;
  Glib::RefPtr<Gio::Cancellable> cancellable = Gio::Cancellable::create();
  Glib::Threads::Mutex mutex;
  std::queue<std::shared_ptr<Task>> result_queue;
  Glib::Dispatcher dispatcher;
};

#endif  // LUMEE_IMAGE_WORKER_H
