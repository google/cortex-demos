/*******************************************************************************
    Copyright 2018 Google LLC

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        https://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*******************************************************************************/

#include "core/thread.hpp"

#ifndef configASSERT
#define configASSERT(...)
#endif

#ifndef threadASSERT
#define threadASSERT configASSERT
#endif

namespace os {

void Thread::run() {
    setup();

    while (1) {
        mainloop();
    }
}

void run_thread(void* params) {
    auto* thread = reinterpret_cast<Thread*>(params);
    thread->run();

    threadASSERT(0);
}

}
