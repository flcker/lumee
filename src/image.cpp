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

#include "image.h"

ImageWorker::ImageWorker() : pool(1) {
  dispatcher.connect(sigc::mem_fun(*this, &ImageWorker::emit_finished));
}

void ImageWorker::load(const std::string& filename,
    Glib::RefPtr<Gio::Cancellable>& cancellable) {
  pool.push(sigc::bind(sigc::mem_fun(*this, &ImageWorker::create_pixbuf),
        filename, cancellable));
}

// Run in a worker thread
void ImageWorker::create_pixbuf(const std::string& filename,
    Glib::RefPtr<Gio::Cancellable>& cancellable) {
  // The task can be cancelled at any time, even before it starts, so check the
  // cancellable both before and after creating the pixbuf
  if (cancellable->is_cancelled())
    return;
  // FIXME: Compare Pixbuf::create_from_file's speed with PixbufLoader
  // FIXME: Check if network-mounted or very large images can stall the thread
  // FIXME: Support animated images
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(filename);
  if (cancellable->is_cancelled())
    return;

  {
    Glib::Threads::Mutex::Lock lock(mutex);
    queue.push(pixbuf);
  }
  dispatcher.emit();
}

// Run in the main thread
void ImageWorker::emit_finished() {
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  {
    Glib::Threads::Mutex::Lock lock(mutex);
    pixbuf = queue.front();
    queue.pop();
  }
  signal_finished.emit(pixbuf);
}
