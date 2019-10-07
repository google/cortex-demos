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

#include "nvic.h"
#include "nrf52/power.hpp"

constexpr uint32_t power_base = 0x40000000;
constexpr uint32_t evt_usbdetected = power_base + 0x11c;
constexpr uint32_t evt_usbremoved = power_base + 0x120;
constexpr uint32_t evt_usbpwrrdy = power_base + 0x124;

constexpr uint32_t usb_regstatus = power_base + 0x438;

namespace {

bool nvic_initialized = false;

void nvic_init_once() {
    if (!nvic_initialized) {
        nvic_init();
        nvic_initialized = true;
    }
}

class DummyEventHandler : public driver::EventHandler {
    public:
        void handle_event(driver::EventInfo* e_info) override {
            (void)e_info;
            ++evt_counter;
        }

        int evt_counter = 0;
};

}

TEST_CASE("Test Power USB Features", "[power,usb]") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    nvic_init_once();

    auto* power = nrf52::Power::request();
    REQUIRE(power);

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

TEST_CASE("Test Power IRQ/Event Handling", "[power,irq]") {
    using nrf52::Power;

    auto& mem = mock::get_global_memory();
    mem.reset();

    nvic_init_once();

    auto* power = Power::request();
    REQUIRE(power);

    constexpr auto power_irqnum = 0;
    CHECK(nvic_dispatch(power_irqnum) >= 0);

    DummyEventHandler dummy_event_handler;
    CHECK(power->add_event_handler(Power::Event::USBDETECTED, &dummy_event_handler) >= 0);

    mem.set_value_at(evt_usbdetected, 0);
    mem.set_value_at(evt_usbpwrrdy, 1);
    CHECK(nvic_dispatch(power_irqnum) >= 0);
    CHECK(dummy_event_handler.evt_counter == 0);

    mem.set_value_at(evt_usbdetected, 1);
    CHECK(nvic_dispatch(power_irqnum) >= 0);
    CHECK(dummy_event_handler.evt_counter == 1);
}
