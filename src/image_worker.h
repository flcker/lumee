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
#include <glibmm/dispatcher.h>
#include <glibmm/threads.h>

#include <queue>

// Loads images in a thread and returns the results asynchronously.
class ImageWorker {
 public:
  // Function that will be called when an image has finished loading or failed
  // to load. If it failed, the pixbuf pointer will be empty.
  typedef sigc::slot<void, Glib::RefPtr<Gdk::Pixbuf>> SlotFinished;

  ImageWorker();
  ~ImageWorker();

  // Loads an image file asynchronously.
  //
  // `slot` will be called when finished. If it's a non-static member function
  // and the class derives from `sigc::trackable`, don't use `sigc::mem_fun()`
  // - it's not thread-safe. Use `std::bind()` or a lambda instead.
  //
  // `scale_size` (optional) is the maximum width and height of the image. It
  // will be scaled if needed.
  void load(const SlotFinished& slot, const std::string& path,
            int scale_size = 0);

  // Cancels the running task and removes all queued tasks.
  void cancel_all();

 private:
  // Task that can be processed.
  struct Task {
    Task(const SlotFinished& slot_finished, const std::string& path,
         int scale_size)
        : slot_finished(slot_finished), path(path), scale_size(scale_size) {}
    Task() {}

    SlotFinished slot_finished;
    std::string path;
    int scale_size = 0;
    Glib::RefPtr<Gdk::Pixbuf> result;
  };

  // Processes a slot and task, passing the result to the main thread.
  void process(const sigc::slot<void, Task&>& slot, Task& task);

  // Does the actual loading of an image file.
  void load_task(Task& task);

  // Removes a task from the result queue and calls its slot.
  void finish_task();

  WorkQueue work_queue;
  Glib::RefPtr<Gio::Cancellable> cancellable = Gio::Cancellable::create();
  Glib::Threads::Mutex mutex;
  std::queue<Task> results;
  Glib::Dispatcher dispatcher;
};

#endif  // LUMEE_IMAGE_WORKER_H
