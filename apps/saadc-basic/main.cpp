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

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

#include "core/freertos_thread.hpp"
#include "core/init.hpp"
#include "driver/timer.hpp"
#include "driver/uart.hpp"
#include "driver/adc.hpp"
#include "gpio.h"
#include "memio.h"

namespace {

uint8_t alloc_buffer[100];

extern "C" void* _sbrk(int incr) {
    (void)incr;
    return alloc_buffer;
}

extern "C" void xPortSysTickHandler(void);

class RTCTickHandler : public driver::EventHandler {
    public:
        void handle_event(driver::EventInfo* e_info) override {
            (void)e_info;
            xPortSysTickHandler();
        }
} rtc_tick_handler;

extern "C" {

    const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

    void xPortPendSVHandler(void);
    void vPortSVCHandler(void);

    void vPortSetupTimerInterrupt(void) {
        auto* rtc = driver::Timer::request_by_id(driver::Timer::ID::RTC0);
        auto rate = rtc->get_rate();
        rtc->set_prescaler(rate / configTICK_RATE_HZ);
        rtc->add_event_handler(0, &rtc_tick_handler);
        rtc->enable_tick_interrupt();
        rtc->start();
    }

}  // extern "C"

class BlinkerThread : public os::ThreadStatic<2 * configMINIMAL_STACK_SIZE> {
    public:
        BlinkerThread() : os::ThreadStatic<2 * configMINIMAL_STACK_SIZE>("BLINK", tskIDLE_PRIORITY + 1) {}
        void setup() override {
            gpio_set_option(0, (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20), GPIO_OPT_OUTPUT);

            counter_ = 0;

            uart_ = driver::UART::request_by_id(driver::UART::ID::UARTE0);
            configASSERT(uart_);
        }

        void mainloop() override {
            ++counter_;
            const auto gpio_n = 17 + (counter_++ & 3);
            gpio_toggle(0, (1 << gpio_n));
            vTaskDelay(pdMS_TO_TICKS(400));
            if (counter_ > 20) {
                uart_->write_str("C\r\n");
                counter_ = 0;
            }
        }
    private:
        int counter_ = 0;
        driver::UART* uart_ = nullptr;
} blinker_thread;

class ADCThread : public os::ThreadStatic<16 * configMINIMAL_STACK_SIZE> {
    public:
        ADCThread() : os::ThreadStatic<16 * configMINIMAL_STACK_SIZE>("ADC", tskIDLE_PRIORITY + 2) {}
        void setup() override {
            uart_ = driver::UART::request_by_id(driver::UART::ID::UARTE0);
            configASSERT(uart_);

            adc_ = driver::ADC::request_by_id(driver::ADC::ID::ADC0);
            configASSERT(adc);
        }

        void mainloop() override {
            vTaskDelay(pdMS_TO_TICKS(2000));
            auto res = adc_->start(3);
            if (res > 0) {
                snprintf(debug_str, sizeof(debug_str), "NRES: %d\r\n", res);
                uart_->write_str(debug_str);
                auto ch0 = adc_->get_result(0, 0);
                auto ch1 = adc_->get_result(1, 0);
                snprintf(debug_str, sizeof(debug_str), "CH0-0: %lu, CH1-0: %lu\r\n", ch0, ch1);
                uart_->write_str(debug_str);

                ch0 = adc_->get_result(0, 1);
                ch1 = adc_->get_result(1, 1);
                snprintf(debug_str, sizeof(debug_str), "CH0-1: %lu, CH1-1: %lu\r\n", ch0, ch1);
                uart_->write_str(debug_str);
            } else {
                uart_->write_str("FAIL\r\n");
            }
        }

    private:
        char debug_str[80];
        driver::UART* uart_;
        driver::ADC* adc_;
} adc_thread;

}  // namespace


int main() {
    os::init();

    blinker_thread.init();
    adc_thread.init();

    auto* uart = driver::UART::request_by_id(driver::UART::ID::UARTE0);

    uart->write_str("Start\r\n");

    vTaskStartScheduler();

    while (1) ;
}
