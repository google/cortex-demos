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

#include <stdint.h>

#define PINCTRL_ENTRY_LIST_START static const struct entry pinconfig_entries[] = {
#define PINCTRL_ENTRY_LIST_END };

#define PINCTRL_DEFINE_BOARD_CONFIG static const struct config board_config = { \
        .n_entries = sizeof(pinconfig_entries) / sizeof(pinconfig_entries[0]), \
                     .entries = pinconfig_entries, \
    }; \
    const struct config* const board_pin_config = &board_config;

namespace pinctrl {

struct entry {
    int function;
    int pin;
};

struct config {
    unsigned int n_entries;
    const struct entry* entries;
};

extern const struct config* const board_pin_config;

// Note, that -1 should not be used as a pin number,
// as this is a way to indicate errors.

int get_pin(int function);

// Request the function to be configured and returns
// the pin for that function.
int request_function(int function);

}  // pinctrl
