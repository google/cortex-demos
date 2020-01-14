/*******************************************************************************
    Copyright 2020 Google LLC

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

#include <list>
#include <functional>
#include <type_traits>

namespace mock {

template<class R, class... Args>
class FunctionStub {
    public:
        FunctionStub() {}
        FunctionStub(const std::function<R(Args...)>& f) {
            (void)f;
        }

        R operator()(Args... args) {
            if (handler_) {
                return handler_(args...);
            }

            if (!rv_list_.empty()) {
                R result{*rv_list_.cbegin()};
                rv_list_.pop_front();
                return result;
            }

            if (rv_initialized_) {
                return return_value_;
            }

            return R();
        }

        void set_return_value(const R& rv) {
            return_value_ = rv;
            rv_initialized_ = true;
        }

        void push_return_value(const R& rv) {
            rv_list_.emplace_back(rv);
        }

        void set_handler(std::function<R(Args...)> handler) {
            handler_ = handler;
        }

    private:
        R return_value_;
        bool rv_initialized_ = false;

        std::list<R> rv_list_;
        std::function<R(Args...)> handler_;

        static_assert(std::is_copy_constructible<R>::value, "Return value must be copy constructible");
};

}  // namespace mock
