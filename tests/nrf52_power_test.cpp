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

#include "third_party/catch2/catch.hpp"

#include "mock_memio.hpp"

#include "nrf52/power.hpp"

constexpr uint32_t power_base = 0x40000000;
constexpr uint32_t evt_usbdetected = power_base + 0x11c;
constexpr uint32_t evt_usbremoved = power_base + 0x120;
constexpr uint32_t evt_usbpwrrdy = power_base + 0x124;

constexpr uint32_t usb_regstatus = power_base + 0x438;

TEST_CASE("Test Power USB Features", "[power,usb]") {
    auto* power = nrf52::Power::request();
    REQUIRE(power);

    auto& mem = mock::get_global_memory();
    mem.reset();

    REQUIRE(!power->is_usb_detected());
    REQUIRE(!power->is_usb_power_ready());

    mem.set_value_at(evt_usbdetected, 1);
    CHECK(power->is_usb_detected());

    mem.set_value_at(evt_usbdetected, 0);
    mem.set_value_at(usb_regstatus, 1);
    CHECK(power->is_usb_detected());

    mem.set_value_at(evt_usbpwrrdy, 1);
    CHECK(power->is_usb_power_ready());

    mem.set_value_at(evt_usbpwrrdy, 0);
    mem.set_value_at(usb_regstatus, 2);
    CHECK(power->is_usb_power_ready());
}
