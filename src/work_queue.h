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

#ifndef LUMEE_WORK_QUEUE_H
#define LUMEE_WORK_QUEUE_H

#include <glibmm/threads.h>

#include <deque>

// Executes work in a thread.
class WorkQueue {
 public:
  ~WorkQueue();

  // Adds a slot to the end of the queue. If this is the first time push() is
  // called for this instance, starts a thread first.
  void push(const sigc::slot<void>& slot);

  // Removes all slots from the queue.
  void clear();

  // Stops processing the queue and waits for the thread to exit.
  void stop();

  // Function to call in the critical section after a work slot is popped from
  // the queue, before the work slot is called.
  sigc::slot<void> slot_popped;

 private:
  // Runs a loop in a thread waiting for slots.
  void run();

  Glib::Threads::Thread* thread = nullptr;
  Glib::Threads::Mutex mutex;
  Glib::Threads::Cond cond;
  std::deque<sigc::slot<void>> slots;
  bool stopping = false;
};

#endif  // LUMEE_WORK_QUEUE_H
