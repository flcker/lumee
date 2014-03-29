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

#include "image_worker.h"
#include "utils.h"

#include <giomm/error.h>

ImageWorker::ImageWorker() {
  // Since a single cancellable is reused, it should be reset before each task.
  work_queue.slot_popped = [this]() { cancellable->reset(); };
  dispatcher.connect(sigc::mem_fun(*this, &ImageWorker::finish_task));
}

ImageWorker::~ImageWorker() {
  cancel_all();
  work_queue.stop();
}

void ImageWorker::load(const SlotFinished& slot, const std::string& path,
                       int scale_size) {
  work_queue.push(sigc::bind(sigc::mem_fun(*this, &ImageWorker::process),
                             sigc::mem_fun(*this, &ImageWorker::load_task),
                             Task(slot, path, scale_size)));
}

// Since the cancellable is reset after a task is popped, `clear()` needs to
// run first. Otherwise, a task could start between `cancel()` and `clear()`.
// This function would need to lock `work_queue`'s mutex to avoid that.
void ImageWorker::cancel_all() {
  work_queue.clear();
  cancellable->cancel();
}

// Runs in a worker thread.
void ImageWorker::process(const sigc::slot<void, Task&>& slot, Task& task) {
  try {
    slot(task);
  } catch (const Gio::Error& error) {
    if (error.code() == Gio::Error::CANCELLED)
      return;
    else throw;
  } catch (const Glib::Error&) {
    task.result.reset();
  }
  {
    Glib::Threads::Mutex::Lock lock(mutex);
    results.push(std::move(task));
  }
  dispatcher.emit();
}

// Runs in a worker thread.
void ImageWorker::load_task(Task& task) {
  // TODO: Compare `Pixbuf::create_from_file()`'s speed with `PixbufLoader`.
  // TODO: Check if network-mounted or very large images can stall the thread.
  // TODO: Support animated images.
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(task.path);
  if (cancellable->is_cancelled())
    throw Gio::Error(Gio::Error::CANCELLED, "");

  if (task.scale_size) {
    double factor = Dimensions(pixbuf).fit(task.scale_size);
    pixbuf = pixbuf->scale_simple(std::round(pixbuf->get_width() * factor),
                                  std::round(pixbuf->get_height() * factor),
                                  Gdk::INTERP_BILINEAR);
    if (cancellable->is_cancelled())
      throw Gio::Error(Gio::Error::CANCELLED, "");
  }
  task.result = pixbuf;
}

// Runs in the main thread.
void ImageWorker::finish_task() {
  Task task;
  {
    Glib::Threads::Mutex::Lock lock(mutex);
    task = results.front();
    results.pop();
  }
  task.slot_finished(task.result);
}
