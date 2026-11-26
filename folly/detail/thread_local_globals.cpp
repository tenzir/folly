/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/detail/thread_local_globals.h>

namespace folly::detail {

namespace {

/// thread_is_dying_flag
///
/// This is a hack. The C runtime library provides no indication that a thread
/// has ended the phase where thread_local variables may be destroyed. Were it
/// to provide that indication, that would be used instead. Relies on library
/// facilities which have signal to mark that a thread is dying.
///
/// In glibc, in the shutdown phase, start_thread first calls __call_tls_dtors,
/// which runs destructors of thread_local variables, and then immediately calls
/// __nptl_deallocate_tsd, which runs destructors of pthread thread-specific
/// variables. There is no other indication of state change in the current
/// thread.
///
/// https://github.com/bminor/glibc/blob/glibc-2.39/nptl/pthread_create.c#L451-L455
///
/// In musl, which has no __cxa_thread_atexit_impl, the C++ runtime destroys
/// thread_local variables from a pthread thread-specific destructor of its
/// own, so they may also be destroyed after the mark. The mark therefore lives
/// in a trivially-destructible thread_local, which stays valid until the thread
/// is gone. A pthread thread-specific value would not do: musl clears the value
/// of every key, with or without a destructor, as __pthread_tsd_run_dtors walks
/// the keys, and so would forget the mark before the later keys' destructors
/// run.
///
/// https://git.musl-libc.org/cgit/musl/tree/src/thread/pthread_key_create.c?h=v1.2.5
thread_local bool thread_is_dying_flag = false;

} // namespace

bool thread_is_dying() {
  return thread_is_dying_flag;
}

void thread_is_dying_mark() {
  thread_is_dying_flag = true;
}

} // namespace folly::detail
