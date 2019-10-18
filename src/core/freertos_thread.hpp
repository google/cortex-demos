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

#pragma once

#include "core/thread.hpp"

#include "cutils.h"

#include "FreeRTOS.h"
#include "task.h"

namespace os {

#if configSUPPORT_STATIC_ALLOCATION

template <uint32_t StackDepth>
class ThreadStatic : public Thread {
    public:
        ThreadStatic(const char* name, UBaseType_t priority) : name_{name}, priority_{priority} {}

        void init() override {
            handle_ = xTaskCreateStatic(
                          os::run_thread, name_, ARRAY_SIZE(stack_buffer_),
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

}
