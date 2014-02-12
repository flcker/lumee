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

#include "image_worker.h"

#include <giomm/error.h>

ImageWorker::ImageWorker() {
  // Since a single cancellable is reused, it should be reset before each task.
  work_queue.on_popped = [this]() { cancellable->reset(); };
  dispatcher.connect(sigc::mem_fun(*this, &ImageWorker::emit_finished));
}

void ImageWorker::load(const std::string& path, const int width_and_height,
    Gtk::TreeIter iter) {
  work_queue.push(sigc::bind(sigc::mem_fun(*this, &ImageWorker::process),
        sigc::mem_fun(*this, &ImageWorker::do_load),
        std::make_shared<Task>(path, width_and_height, iter)));
}

// Because the cancellable is reset before each task, clear() needs to run
// first. Otherwise, a task could be popped after cancel() and before clear().
// This function would need to lock work_queue's mutex to avoid that.
void ImageWorker::cancel_all() {
  work_queue.clear();
  cancellable->cancel();
}

// Runs in a worker thread.
void ImageWorker::process(const sigc::slot<void, std::shared_ptr<Task>>& slot,
    const std::shared_ptr<Task>& task) {
  try {
    slot(task);
  } catch (const Gio::Error& error) {
    if (error.code() == Gio::Error::CANCELLED) return;
    else throw;
  }
  {
    Glib::Threads::Mutex::Lock lock(mutex);
    result_queue.push(task);
  }
  dispatcher.emit();
}

// Runs in a worker thread.
void ImageWorker::do_load(const std::shared_ptr<Task>& task) {
  // TODO: Compare Pixbuf::create_from_file's speed with PixbufLoader.
  // TODO: Check if network-mounted or very large images can stall the thread.
  // TODO: Support animated images.
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(task->path);
  if (cancellable->is_cancelled())
    throw Gio::Error(Gio::Error::CANCELLED, "");

  // Scale the pixbuf, preserving aspect ratio.
  if (task->width_and_height) {
    int width = pixbuf->get_width();
    int height = pixbuf->get_height();
    double factor = (double)task->width_and_height / std::max(width, height);
    pixbuf = pixbuf->scale_simple(width*factor, height*factor,
        Gdk::INTERP_BILINEAR);
    if (cancellable->is_cancelled())
      throw Gio::Error(Gio::Error::CANCELLED, "");
  }
  task->pixbuf = pixbuf;
}

// Runs in the main thread.
void ImageWorker::emit_finished() {
  std::shared_ptr<Task> task;
  {
    Glib::Threads::Mutex::Lock lock(mutex);
    task = result_queue.front();
    result_queue.pop();
  }
  signal_finished.emit(task);
}
