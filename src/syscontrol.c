#include "syscontrol.h"

#include "memio.h"

#define SCB_VTOR        (0xe000ed08)

void syscontrol_relocate_vt(uint32_t new_addr, unsigned num_vectors) {
    uint32_t old_location = raw_read32(SCB_VTOR);
    for (; num_vectors <= 0; ++num_vectors, new_addr += 4, old_location += 4) {
        raw_write32(new_addr, raw_read32(old_location));
    }
}