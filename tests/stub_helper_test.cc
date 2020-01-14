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

#include "stub_helper.hpp"

#include "third_party/catch2/catch.hpp"

TEST_CASE("Test Stub Helper API") {
    mock::FunctionStub<int, int> fs{};

    CHECK(fs(10) == int());

    fs.set_return_value(42);
    CHECK(fs(12) == 42);
}
