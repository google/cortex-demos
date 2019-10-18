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

#include "nrf52/pinctrl.hpp"

namespace pinctrl {

// *INDENT-OFF*
PINCTRL_ENTRY_LIST_START
    {
        .function = function::UARTE0_RXD,
        .pin = 8,
    },
    {
        .function = function::UARTE0_CTS,
        .pin = 7,
    },
    {
        .function = function::UARTE0_RTS,
        .pin = 5,
    },
    {
        .function = function::UARTE0_TXD,
        .pin = 6,
    },
PINCTRL_ENTRY_LIST_END;
// *INDENT-ON*

PINCTRL_DEFINE_BOARD_CONFIG;

};
