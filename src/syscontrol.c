#include "syscontrol.h"

#include "memio.h"

#define SCB_BASE        (0xe000ed00)

#define SCB_ICSR        (SCB_BASE + 0x04)
#define SCB_VTOR        (SCB_BASE + 0x08)

void syscontrol_relocate_vt(ubase_t new_addr, unsigned num_vectors) {
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
