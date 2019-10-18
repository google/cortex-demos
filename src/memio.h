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

#pragma once

#ifndef TEST_MEMIO
#   define raw_write32(addr, value)     *((volatile uint32_t*)(addr)) = (value)
#   define raw_writeptr(addr, ptr)     *((volatile uint32_t*)(addr)) = (uint32_t)(ptr)
#   define raw_write16(addr, value)     *((volatile uint16_t*)(addr)) = (value)
#   define raw_write8(addr, value)     *((volatile uint8_t*)(addr)) = (value)

#   define raw_read32(addr)     *((volatile uint32_t*)(addr))
#   define raw_readptr(addr)     (void*)(*((volatile uint32_t*)(addr)))
#   define raw_read16(addr)     *((volatile uint16_t*)(addr))
#   define raw_read8(addr)     *((volatile uint8_t*)(addr))
#else  /* !defined(TEST_MEMIO) */

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void raw_write32(uint32_t addr, uint32_t value);
void raw_writeptr(uint32_t addr, void* ptr);
void raw_write16(uint32_t addr, uint16_t value);
void raw_write8(uint32_t addr, uint8_t value);

void* raw_readptr(uint32_t addr);
uint32_t raw_read32(uint32_t addr);
uint16_t raw_read16(uint16_t addr);
uint8_t raw_read8(uint8_t addr);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* defined TEST_MEMIO */

#ifdef __cplusplus
#define __auto_type auto
#endif

#define wait_mask_le32(addr, mask) while (!(raw_read32((addr)) & mask))

#define raw_setbits_le32(addr, mask) do {\
        __typeof__(addr) _addr = (addr);\
        raw_write32(_addr, raw_read32(_addr) | (mask));\
    } while (0)

#define raw_clrbits_le32(addr, mask) do {\
        __typeof__(addr) _addr = (addr);\
        raw_write32(_addr, raw_read32(_addr) & ~(mask));\
    } while (0)

#define raw_set_masked(addr, mask, value) do {\
        __typeof__(addr) _addr = (addr);\
        __auto_type _cvalue = raw_read32(_addr);\
        _cvalue &= ~(mask);\
        _cvalue |= mask & (value);\
        raw_write32(_addr, _cvalue);\
    } while (0)
