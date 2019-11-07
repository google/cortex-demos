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

#include "third_party/catch2/catch.hpp"

#include "mock_memio.hpp"

#include "nrf52/usbd.hpp"

constexpr uint32_t usbd_base = 0x4002'7000;
constexpr uint32_t usbd_enable = usbd_base + 0x500;
constexpr uint32_t usbd_addr = usbd_base + 0x470;
constexpr uint32_t usbd_pullup = usbd_base + 0x504;


TEST_CASE("USBD Basics") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    nrf52::USBD usb;
    usb.enable();
    CHECK(mem.get_value_at(usbd_enable) == 1);

    CHECK(usb.get_irq_num() == 39);

    mem.set_value_at(usbd_addr, 56);
    CHECK(usb.get_addr() == 56);

    usb.pullup(true);
    CHECK(mem.get_value_at(usbd_pullup) == 1);

    usb.pullup(false);
    CHECK(mem.get_value_at(usbd_pullup) == 0);
}
