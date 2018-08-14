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

#include <nvic.h>

volatile int counter = 0;

void simple_handler(void) {
    ++counter;
}

int main(void) {
    nvic_init();
    nvic_set_handler(IRQ_PENDSV, simple_handler);
    nvic_enable_irqs();

    nvic_set_handler(1, simple_handler);
    nvic_enable_irq(1);

    nvic_irqset(IRQ_PENDSV);
    nvic_irqset(1);
    while (1);
}
