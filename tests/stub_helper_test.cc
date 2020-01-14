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

int my_func(int a, int b) {
    return a + b;
}

TEST_CASE("Test Stub Helper API") {
    mock::FunctionStub<int, int> fs{};

    CHECK(fs(10) == int());

    fs.set_return_value(42);
    CHECK(fs(12) == 42);

    fs.push_return_value(89);
    fs.push_return_value(105);
    CHECK(fs(234) == 89);
    CHECK(fs(2323) == 105);
    CHECK(fs(5462) == 42);
    CHECK(fs(562) == 42);

    fs.set_handler([](int v) {
        return 2 * v + 1;
    });
    CHECK(fs(12) == (2 * 12 + 1));
    CHECK(fs(432) == (2 * 432 + 1));
}

TEST_CASE("Test construction from function object") {
    mock::FunctionStub fs{std::function{my_func}};

    CHECK(fs(23, 43) == int());
}
