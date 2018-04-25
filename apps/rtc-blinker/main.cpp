#include "gpio.h"
#include "nvic.h"
#include "driver/timer.hpp"

#include "memio.h"

namespace {

int counter = 0;

void toggle_leds(int) {
    auto gpio_n = 17 + (counter++ & 3);
    gpio_toggle(0, (1 << gpio_n));
}

}

int main(void) {
    nvic_init();

    gpio_set_option(0, (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20), GPIO_OPT_OUTPUT);

    auto* rtc = driver::Timer::get_by_id(driver::Timer::ID::RTC0);

    rtc->stop();
    rtc->set_prescaler(0xffd);
    rtc->add_event_handler(0, toggle_leds);
    rtc->enable_tick_interrupt();
    rtc->start();

    while (1);
}
