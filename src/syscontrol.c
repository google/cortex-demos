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

#include "syscontrol.h"

#include "memio.h"

#define SCB_BASE        (0xe000ed00)

#define SCB_ICSR        (SCB_BASE + 0x04)
#define SCB_VTOR        (SCB_BASE + 0x08)

void syscontrol_relocate_vt(uintptr_t new_addr, unsigned num_vectors) {
    uint32_t old_location = raw_read32(SCB_VTOR);
    /* FIXME: this does not check the alignment */
    /* Adjust for the stack top location */
    ++num_vectors;
    for (uint32_t addr = (uint32_t)new_addr;
         num_vectors > 0; --num_vectors, addr += 4, old_location += 4) {
        raw_write32(addr, raw_read32(old_location));
    }

    raw_write32(SCB_VTOR, new_addr);
}
