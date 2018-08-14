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

namespace periph {

namespace {

constexpr uint32_t kPeriphBase = 0x40000000;

}  // namespace

constexpr uint32_t id_to_base(unsigned int p_id) {
    return kPeriphBase + p_id * 0x1000;
}

}  // namespace periph
