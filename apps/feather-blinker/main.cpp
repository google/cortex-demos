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
#include "driver/timer.hpp"
#include "driver/uart.hpp"
#include "driver/adc.hpp"
#include "gpio.h"
#include "memio.h"
#include "nvic.h"

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
    auto* rtc = driver::Timer::get_by_id(driver::Timer::ID::RTC0);
    auto rate = rtc->get_rate();
    rtc->set_prescaler(rate / configTICK_RATE_HZ);
    rtc->add_event_handler(0, &rtc_tick_handler);
    rtc->enable_tick_interrupt();
    rtc->start();
}

// TODO: Move to non-app specific location
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

}  // extern "C"

class BlinkerThread : public os::ThreadStatic<2 * configMINIMAL_STACK_SIZE> {
    public:
        BlinkerThread() : os::ThreadStatic<2 * configMINIMAL_STACK_SIZE>("BLINK", tskIDLE_PRIORITY + 1) {}
        void setup() override {
            gpio_set_option(0, (1 << 17) | (1 << 19), GPIO_OPT_OUTPUT);

            counter_ = 0;

            uart_ = driver::UART::request_by_id(driver::UART::ID::UARTE0);
            configASSERT(uart_);
        }

        void mainloop() override {
            ++counter_;
            const auto gpio_n = counter_ & 1 ? 17 : 19;
            gpio_toggle(0, (1 << gpio_n));
            vTaskDelay(pdMS_TO_TICKS(400));
        }
    private:
        int counter_ = 0;
        driver::UART* uart_ = nullptr;
} blinker_thread;

}  // namespace


int main() {
    nvic_init();

    nvic_set_handler(IRQ_SVCALL, vPortSVCHandler);
    nvic_set_handler(IRQ_PENDSV, xPortPendSVHandler);

    blinker_thread.init();

    vTaskStartScheduler();

    while (1) ;
}
