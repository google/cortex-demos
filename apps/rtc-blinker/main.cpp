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

#include "gpio.h"
#include "nvic.h"
#include "driver/timer.hpp"

#include "memio.h"

namespace {

class ToggleLEDsHandlers : public driver::EventHandler {
    public:
        void handle_event(driver::EventInfo* e_info) override {
            (void)e_info;
            auto gpio_n = 17 + (counter++ & 3);
            gpio_toggle(0, (1 << gpio_n));
        }

    private:
        unsigned counter = 0;
} toggle_leds_handler;

}

int main(void) {
    nvic_init();

    gpio_set_option(0, (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20), GPIO_OPT_OUTPUT);

    auto* rtc = driver::Timer::request_by_id(driver::Timer::ID::RTC0);

    rtc->stop();
    rtc->set_prescaler(0xffd);
    rtc->add_event_handler(0, &toggle_leds_handler);
    rtc->enable_tick_interrupt();
    rtc->start();

    while (1);
}
