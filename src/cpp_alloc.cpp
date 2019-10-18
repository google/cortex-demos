/*******************************************************************************
    Copyright 2019 Google LLC

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

#include <cstdint>
#include <cstddef>

void __attribute__((weak)) operator delete (void* ptr) {
    (void)ptr;
    while (1);
}

void __attribute__((weak)) operator delete (void* ptr, unsigned) {
    (void)ptr;
    while (1);
}

void __attribute__((weak)) operator delete (void* ptr, long unsigned) {
    (void)ptr;
    while (1);
}

void* __attribute__((weak)) operator new (size_t count) throw() {
    (void)count;
    return nullptr;
}

void* __attribute__((weak)) operator new[](size_t count) throw() {
    (void)count;
    return nullptr;
}

namespace std {

void __attribute__((weak)) __throw_bad_alloc() {
    while (1);
}

}
