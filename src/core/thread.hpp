#pragma once

#include <cstdint>

#ifndef threadASSERT
#define threadASSERT configASSERT
#endif

namespace os {

void run_thread(void*);

class Thread {
    public:
        Thread() {}
        Thread(const Thread&) = delete;
        virtual ~Thread() {}

        virtual void init() = 0;
        virtual void run();
        virtual void setup() = 0;
        virtual void mainloop() = 0;
};

}  // namespace os
