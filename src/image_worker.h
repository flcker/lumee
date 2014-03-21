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

#ifndef LUMEE_IMAGE_WORKER_H
#define LUMEE_IMAGE_WORKER_H

#include "work_queue.h"

#include <gdkmm/pixbuf.h>
#include <gtkmm/treeiter.h>
#include <glibmm/dispatcher.h>
#include <glibmm/threads.h>

#include <queue>

// Loads images in a thread and returns the results asynchronously. The result
// of each task started by `load()` will be emitted in `signal_finished`.
class ImageWorker {
 public:
  // Task that can be processed.
  struct Task {
    Task(const std::string& path, int scale_size, const Gtk::TreeIter& iter)
        : path(path), scale_size(scale_size), iter(iter) {}
    Task() {}

    // Path to the image file.
    std::string path;

    // Size to scale the image to, if any (assumes an equal width and height).
    int scale_size = 0;

    // TODO: This doesn't belong here, but `ImageList` needs it.
    Gtk::TreeIter iter;

    // Result of the finished task. If it failed, the pointer will be empty.
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  };

  ImageWorker();
  ~ImageWorker();

  // Loads an image file asynchronously (see `Task`).
  void load(const std::string& path, int scale_size = 0,
            const Gtk::TreeIter& iter = Gtk::TreeIter());

  // Cancels the running task and removes all queued tasks.
  void cancel_all();

  // Emitted when an image has finished loading or failed to load.
  sigc::signal<void, Task> signal_finished;

 private:
  // Processes a slot and task, passing the result to the main thread.
  void process(const sigc::slot<void, Task&>& slot, Task& task);

  // Does the actual loading of an image file.
  void load_task(Task& task);

  // Pops a task from the result queue and emits the signal.
  void finish_task();

  WorkQueue work_queue;
  Glib::RefPtr<Gio::Cancellable> cancellable = Gio::Cancellable::create();
  Glib::Threads::Mutex mutex;
  std::queue<Task> results;
  Glib::Dispatcher dispatcher;
};

#endif  // LUMEE_IMAGE_WORKER_H
