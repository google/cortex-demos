#pragma once

#include <cstdint>
#include <map>
#include <tuple>
#include <vector>

namespace mock {

class Memory {
    public:
        enum class Op {
            READ8,
            READ16,
            READ32,
            WRITE8,
            WRITE16,
            WRITE32,
        };

        using JournalEntry = std::tuple<Op, uint32_t, uint32_t>;
        using JournalT = std::vector<JournalEntry>;

        const JournalT& get_journal() const;

        void reset();

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
        mutable JournalT journal_;
};

Memory& get_global_memory();

}  // namespace
