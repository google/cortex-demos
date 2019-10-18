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

#include "core/freertos_thread.hpp"
#include "core/init.hpp"
#include "clk.h"
#include "gpio.h"
#include "memio.h"
#include "nvic.h"

#include "sam4s/clk.h"
#include "sam4s/gpio.h"

namespace {

extern "C" void xPortSysTickHandler(void);

extern "C" {

    const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

    void xPortPendSVHandler(void);
    void vPortSVCHandler(void);

    // TODO: Move to non-app specific location
    void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                       StackType_t** ppxIdleTaskStackBuffer,
                                       uint32_t* pulIdleTaskStackSize) {
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
            gpio_set_option(led0_port, led0_mask, GPIO_OPT_OUTPUT);
        }

        void mainloop() override {
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_toggle(led0_port, led0_mask);
            vTaskDelay(pdMS_TO_TICKS(1000));
            gpio_toggle(led0_port, led0_mask);
            vTaskDelay(pdMS_TO_TICKS(750));
            gpio_toggle(led0_port, led0_mask);
        }

    private:
        static constexpr auto led0_port = GPIO_PORTC;
        static constexpr auto led0_mask = (1 << 23);
} blinker_thread;

}  // namespace

unsigned int g_cpu_clock_hz;

int main() {
    clk_request(SAM4S_CLK_XTAL_EXT);
    clk_request(SAM4S_CLK_HF_CRYSTAL);
    clk_request(SAM4S_CLK_MAINCK);
    /*
    sam4s_set_crystal_frequency(12 * 1000 * 1000);
    unsigned int pll_rate = clk_request_rate(SAM4S_CLK_PLLACK, 80*1000*1000);

    if (pll_rate) {
        clk_request_option(SAM4S_CLK_MCK, SAM4S_CLK_PLLACK);
    }
    */

    g_cpu_clock_hz = clk_get_rate(SAM4S_CLK_MCK);

    os::init();
    nvic_set_handler(IRQ_SYSTICK, xPortSysTickHandler);

    blinker_thread.init();

    vTaskStartScheduler();

    while (1) ;
}
