#pragma once

#include <cstdint>
#include <map>

namespace mock {

class Memory {
    public:
        uint32_t read32(uint32_t addr) const;
        uint16_t read16(uint32_t addr) const;
        uint8_t read8(uint32_t addr) const;

        void write32(uint32_t addr, uint32_t value);
        void write16(uint32_t addr, uint16_t value);
        void write8(uint32_t addr, uint8_t value);

        void set_value_at(uint32_t addr, uint32_t value);
        uint32_t get_value_at(uint32_t addr);

    private:
        std::map<uint32_t, uint32_t> mem_map_;
};

}  // namespace
