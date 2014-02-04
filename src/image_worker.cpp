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

ImageWorker::ImageWorker() : pool(1) {
  dispatcher.connect(sigc::mem_fun(*this, &ImageWorker::emit_finished));
}

void ImageWorker::load(const std::shared_ptr<ImageTask>& task) {
  // TODO: Can we avoid using a shared pointer here?
  pool.push(sigc::bind(sigc::mem_fun(*this, &ImageWorker::create_pixbuf),
        task));
}

// Run in a worker thread
void ImageWorker::create_pixbuf(const std::shared_ptr<ImageTask>& task) {
  // The task can be cancelled at any time, including before it starts, so
  // check the cancellable at multiple points
  if (task->cancellable->is_cancelled()) return;

  // TODO: Compare Pixbuf::create_from_file's speed with PixbufLoader
  // TODO: Check if network-mounted or very large images can stall the thread
  // TODO: Support animated images
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(task->path);
  if (task->cancellable->is_cancelled()) return;

  // Scale the pixbuf, preserving aspect ratio
  if (task->width_and_height) {
    int width = pixbuf->get_width();
    int height = pixbuf->get_height();
    double factor = (double)task->width_and_height / std::max(width, height);
    pixbuf = pixbuf->scale_simple(width*factor, height*factor,
        Gdk::INTERP_BILINEAR);
    if (task->cancellable->is_cancelled()) return;
  }

  task->pixbuf = pixbuf;
  {
    Glib::Threads::Mutex::Lock lock(mutex);
    queue.push(task);
  }
  dispatcher.emit();
}

// Run in the main thread
void ImageWorker::emit_finished() {
  std::shared_ptr<ImageTask> task;
  {
    Glib::Threads::Mutex::Lock lock(mutex);
    task = queue.front();
    queue.pop();
  }
  signal_finished.emit(task);
}
