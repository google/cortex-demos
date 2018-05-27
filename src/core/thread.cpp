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
