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

#include "work_queue.h"

WorkQueue::~WorkQueue() {
  if (thread) {
    {
      Glib::Threads::Mutex::Lock lock(mutex);
      exiting = true;
    }
    cond.broadcast();
    thread->join();
  }
}

void WorkQueue::push(const sigc::slot<void>& slot) {
  if (!thread)
    thread = Glib::Threads::Thread::create(sigc::mem_fun(*this,
          &WorkQueue::run));
  {
    Glib::Threads::Mutex::Lock lock(mutex);
    deque.push_back(slot);
  }
  cond.signal();
}

void WorkQueue::clear() {
  Glib::Threads::Mutex::Lock lock(mutex);
  deque.clear();
}

void WorkQueue::run() {
  while (true) {
    sigc::slot<void> slot;
    {
      Glib::Threads::Mutex::Lock lock(mutex);
      while (!exiting && deque.empty())
        cond.wait(mutex);
      if (exiting) break;

      slot = deque.front();
      deque.pop_front();
      on_popped();
    }
    slot();
  }
}
