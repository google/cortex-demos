#pragma once

#include <cstdint>

#include "cutils.h"

#include "FreeRTOS.h"
#include "task.h"

#ifndef threadASSERT
#define threadASSERT configASSERT
#endif

namespace os {

namespace {

void run_thread(void*);

}  // namespace

class Thread {
    public:
        Thread() {}
        Thread(const Thread&) = delete;
        virtual ~Thread() {}

        virtual void init() = 0;
        virtual void run() = 0;
};

#if configSUPPORT_STATIC_ALLOCATION

template <uint32_t StackDepth>
class ThreadStatic : public Thread {
    public:
        ThreadStatic(const char* name, UBaseType_t priority) : name_{name}, priority_{priority} {}

        void init() override {
            handle_ = xTaskCreateStatic(
                    run_thread, name_, ARRAY_SIZE(stack_buffer_),
                    this, priority_, stack_buffer_, &task_);

            threadASSERT(handle_);
        }

    private:
        const char* name_;
        UBaseType_t priority_ = 0;
        StaticTask_t task_;
        TaskHandle_t handle_;

        StackType_t stack_buffer_[StackDepth];
};

#endif  // configSUPPORT_STATIC_ALLOCATION

namespace {

void run_thread(void* params) {
    auto* thread = reinterpret_cast<Thread*>(params);
    thread->run();

    threadASSERT(0);
}

}  // namespace


}  // namespace os
