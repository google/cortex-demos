/*******************************************************************************
    Copyright 2018,2019 Google LLC

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

#include "mock_memio.hpp"

#include <algorithm>

namespace mock {

namespace {

/**
 * @brief Global Virtual Memory object that handles all Memory IO operations in tests.
 */
mock::Memory g_memory;

extern "C" void raw_write32(uint32_t addr, uint32_t value) {
    g_memory.write32(addr, value);
}

extern "C" uint32_t raw_read32(uint32_t addr) {
    return g_memory.read32(addr);
}

extern "C" void raw_writeptr(uint32_t addr, void* ptr) {
    g_memory.writeptr(addr, ptr);
}

extern "C" void* raw_readptr(uint32_t addr) {
    return g_memory.readptr(addr);
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
    const auto& it = addr_handler_map_.find(addr);
    if (it != addr_handler_map_.end()) {
        uint32_t old_value = get_value_at(addr, 0);
        ret = it->second->read32(addr, old_value);
    } else {
        const auto it = mem_map_.find(addr);
        if (it != mem_map_.end()) {
            ret = it->second;
        }
    }

    return ret;
}

void Memory::write32(uint32_t addr, uint32_t value) {
    journal_.push_back(std::make_tuple(Memory::Op::WRITE32, addr, value));
    priv_write32(addr, value);
}

void Memory::writeptr(uint32_t addr, void* ptr) {
    mem_ptr_map_[addr] = ptr;
}

void* Memory::readptr(uint32_t addr) const {
    auto it = mem_ptr_map_.find(addr);
    if (it != mem_ptr_map_.end()) {
        return it->second;
    }

    return nullptr;
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

uint32_t Memory::get_value_at(uint32_t addr) const {
    return get_value_at(addr, 0);
}

void* Memory::get_ptr_at(uint32_t addr) const {
    void* res = nullptr;
    const auto& it = mem_ptr_map_.find(addr);
    if (it != mem_ptr_map_.end()) {
        res = it->second;
    }

    return res;
}

void Memory::set_ptr_at(uint32_t addr, void* ptr) {
    mem_ptr_map_[addr] = ptr;
}

uint32_t Memory::get_value_at(uint32_t addr, uint32_t default_value) const {
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

unsigned int Memory::get_op_count(Op op) const {
    return std::count_if(journal_.begin(), journal_.end(), [&](JournalEntry & entry) -> bool{
        return std::get<0>(entry) == op;
    });
}

unsigned int Memory::get_op_count(Op op, uint32_t addr) const {
    return std::count_if(journal_.begin(), journal_.end(), [&](JournalEntry & entry) -> bool{
        return std::get<0>(entry) == op && std::get<1>(entry) == addr;
    });
}

void Memory::reset() {
    mem_map_.clear();
    mem_ptr_map_.clear();
    journal_.clear();
    addr_handler_map_.clear();
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

void Memory::print_journal() const {
    for (const auto& entry : journal_)  {
        std::string op = "XX";
        switch (std::get<0>(entry)) {
        case Memory::Op::WRITE32:
            op = "W32";
            break;
        case Memory::Op::READ32:
            op = "R32";
        default:
            break;
        }

        std::cout << op << ": " << std::hex << std::get<1>(entry) << " " << std::get<2>(entry) << std::endl;
    }
}

void Memory::print_map() const {
    for (const auto& item : mem_map_) {
        std::cout << std::hex << item.first << " => " << item.second << std::endl;
    }
}

Memory& get_global_memory() {
    return g_memory;
}

void IOHandlerStub::set_mem_value(uint32_t addr, uint32_t value) {
    mem_->set_value_at(addr, value);
}

uint32_t IOHandlerStub::get_mem_value(uint32_t addr) const {
    return mem_->get_value_at(addr, 0);
}

}  // namespace mock
