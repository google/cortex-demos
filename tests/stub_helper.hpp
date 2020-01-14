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

#include <functional>

namespace mock {

template<class R, class... Args>
class FunctionStub {
    public:
        FunctionStub() {}

        R operator()(Args... args) {
            if (rv_initialized_) {
                return return_value_;
            }

            return R();
        }

        void set_return_value(const R& rv) {
            return_value_ = rv;
            rv_initialized_ = true;
        }

    private:
        R return_value_;
        bool rv_initialized_ = false;
};

}  // namespace mock
