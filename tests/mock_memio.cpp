#include "mock_memio.hpp"

namespace mock {

namespace {

mock::Memory g_memory;

extern "C" void raw_write32(uint32_t addr, uint32_t value) {
    g_memory.write32(addr, value);
}

extern "C" uint32_t raw_read32(uint32_t addr) {
    return g_memory.read32(addr);
}

}  // namespace

void Memory::priv_write32(uint32_t addr, uint32_t value) {
    const auto& it = addr_handler_map_.find(addr);
    if (it != addr_handler_map_.end()) {
        uint32_t old_value = get_value_at(addr, 0);
        mem_map_[addr] = it->second->write32(addr, old_value, value);
    } else {
        mem_map_[addr] = value;
    }
}

uint32_t Memory::priv_read32(uint32_t addr) const {
    uint32_t ret = 0;
    const auto it = mem_map_.find(addr);
    if (it != mem_map_.end()) {
        ret = it->second;
    }

    return ret;
}

void Memory::write32(uint32_t addr, uint32_t value) {
    journal_.push_back(std::make_tuple(Memory::Op::WRITE32, addr, value));
    priv_write32(addr, value);
}

void Memory::write16(uint32_t addr, uint16_t value) {
    journal_.push_back(std::make_tuple(Memory::Op::WRITE16, addr, value));
    const auto write_addr = addr & (~3);
    uint32_t old_value = priv_read32(write_addr);
    auto new_value = (old_value & (~(0xffff << (8 * (addr & 2)))))
        | (value << (8 * (addr & 2)));

    priv_write32(write_addr, new_value);
}

uint32_t Memory::read32(uint32_t addr) const {
    uint32_t res = priv_read32(addr);

    journal_.push_back(std::make_tuple(Memory::Op::READ32, addr, res));

    return res;
}

// NOTE: This does not allow unaligned reads
uint16_t Memory::read16(uint32_t addr) const {
    const auto lookup_addr = addr & (~3);
    uint32_t res = priv_read32(lookup_addr);
    res >>= ((addr & 2) ? 16 : 0);

    journal_.push_back(std::make_tuple(Memory::Op::READ16, addr, res));

    return res;
}

uint8_t Memory::read8(uint32_t addr) const {
    const auto lookup_addr = addr & (~3);
    uint32_t res = priv_read32(lookup_addr);
    res >>= (8 * (addr & 3));

    journal_.push_back(std::make_tuple(Memory::Op::READ8, addr, res));

    return res;
}

void Memory::set_value_at(uint32_t addr, uint32_t value) {
    mem_map_[addr] = value;
}

uint32_t Memory::get_value_at(uint32_t addr) {
    return mem_map_[addr];
}

uint32_t Memory::get_value_at(uint32_t addr, uint32_t default_value) {
    auto result = default_value;
    const auto& it = mem_map_.find(addr);
    if (it != mem_map_.end()) {
        result = it->second;
    }

    return result;
}

const Memory::JournalT& Memory::get_journal() const {
    return journal_;
}

void Memory::reset() {
    mem_map_.clear();
    journal_.clear();
}

void Memory::set_addr_io_handler(uint32_t addr, IOHandlerStub* io_handler) {
    addr_handler_map_.insert(std::make_pair(addr, io_handler));
    io_handler->set_memory(this);
}

void Memory::set_addr_io_handler(uint32_t range_start, uint32_t range_end, IOHandlerStub* io_handler) {
    for (auto addr = range_start; addr < range_end; addr += sizeof(addr)) {
        addr_handler_map_.insert(std::make_pair(addr, io_handler));
    }
    io_handler->set_memory(this);
}

Memory& get_global_memory() {
    return g_memory;
}

}  // namespace mock
