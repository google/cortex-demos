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
constexpr uint32_t usbd_eventcause = usbd_base + 0x400;


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

    SECTION("USB Event Cause") {
        uint32_t events = (1 << 11) | (1 << 8);
        mem.set_value_at(usbd_eventcause, events);
        mock::W1CStub w1c;
        mem.set_addr_io_handler(usbd_eventcause, &w1c);

        auto evt_result = usb.get_event_cause();
        // All Events returned and cleared
        CHECK(evt_result == events);
        CHECK(mem.get_value_at(usbd_eventcause) == 0);

        using evt = nrf52::USBD::EventCause;

        mem.set_value_at(usbd_eventcause, events);
        evt_result = usb.get_event_cause(evt::RESUME);
        CHECK(evt_result == 0);
        CHECK(mem.get_value_at(usbd_eventcause) == events);

        // One event was read and cleared
        evt_result = usb.get_event_cause(evt::READY);
        CHECK(evt_result == evt::READY);
        CHECK(mem.get_value_at(usbd_eventcause) == evt::SUSPEND);

        // All events read, none cleared
        mem.set_value_at(usbd_eventcause, events);
        evt_result = usb.get_event_cause(evt::MASK_ALL, false);
        CHECK(evt_result == events);
        CHECK(mem.get_value_at(usbd_eventcause) == events);

        // One event read, but not cleared
        mem.set_value_at(usbd_eventcause, events);
        evt_result = usb.get_event_cause(evt::READY, false);
        CHECK(evt_result == evt::READY);
        CHECK(mem.get_value_at(usbd_eventcause) == events);
    }
}
