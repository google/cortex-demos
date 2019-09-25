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

#include "pinctrl.hpp"

namespace pinctrl {

int get_pin(int function) {
    if (!board_pin_config) {
        return -1;
    }

    auto* c_entry = board_pin_config->entries;
    for (unsigned int i = 0; i < board_pin_config->n_entries; ++i) {
        if (c_entry->function == function) {
            return c_entry->pin;
        }

        ++c_entry;
    }

    return -1;
}

}  // namespace pinctrl
