#include "mock_memio.hpp"

namespace mock {

void Memory::write32(uint32_t addr, uint32_t value) {
    mem_map_[addr] = value;
}

void Memory::write16(uint32_t addr, uint16_t value) {
    const auto write_addr = addr & (~3);
    uint32_t old_value = 0;
    const auto it = mem_map_.find(write_addr);
    if (it != mem_map_.end()) {
        old_value = it->second;
    }

    mem_map_[write_addr] = (old_value & (~(0xffff << (8 * (addr & 2)))))
        | (value << (8 * (addr & 2)));
}

uint32_t Memory::read32(uint32_t addr) const {
    const auto it = mem_map_.find(addr);
    if (it != mem_map_.end()) {
        return it->second;
    }

    return 0;
}

// NOTE: This does not allow unaligned reads
uint16_t Memory::read16(uint32_t addr) const {
    const auto lookup_addr = addr & (~3);
    const auto it = mem_map_.find(lookup_addr);
    uint16_t res = 0;
    if (it != mem_map_.end()) {
        res = it->second >> ((addr & 2) ? 16 : 0);
    }

    return res;
}

uint8_t Memory::read8(uint32_t addr) const {
    const auto lookup_addr = addr & (~3);
    const auto it = mem_map_.find(lookup_addr);
    uint8_t res = 0;
    if (it != mem_map_.end()) {
        res = it->second >> (8 * (addr & 3));
    }

    return res;
}


}  // namespace mock
