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

/**
 * @file
 *
 * Utilities for mocking memory IO access. These are main tools for simulating
 * hardware in tests.
 */

#pragma once

#include <iostream>
#include <cstdint>
#include <list>
#include <map>
#include <tuple>
#include <vector>

namespace mock {

class Memory;

/**
 * @brief Base class for more complex IO handlers.
 *
 * The default behavior is the same as not having the custom handler at all,
 * which makes this class by itself not very useful. The derived classes should
 * override at least one of read32(), write32() methods.
 *
 * The objects typically don't need their own value storage, because the values
 * written are still stored in Virtual Memory.
 *
 * FIXME: The API for accessing the global virtual memory is a little awkward at
 * the moment. If the implementation uses global virtual memory object, the user
 * needs to know that and they need to initialize mem_ value. This is not ideal.
 */
class IOHandlerStub {
    public:
        /**
         * @brief Handle write at 32 bit aligned virtual memory address.
         *
         * @param[in] addr Virtual memory address. 32 bit aligned.
         * @param[in] old_value Value currently stored at that address.
         * @param[in] new_value Value that is being written
         *
         * @return The new value that should be stored in the virtual memory at addr.
         */
        virtual uint32_t write32(uint32_t addr, uint32_t old_value, uint32_t new_value) {
            (void)addr;
            (void)old_value;
            return new_value;
        }

        /**
         * @brief Handle read from 32 bit aligned virtual memory address.
         *
         * @param[in]
         * @param[in] addr Virtual memory address. 32 bit aligned.
         *
         * @return The value that should be returned to the reader.
         */
        virtual uint32_t read32(uint32_t addr, uint32_t value) {
            (void)addr;
            return value;
        }

        /**
         * @brief Setter for the mem_ field.
         */
        void set_memory(Memory* mem) {
            mem_ = mem;
        }

    protected:
        /**
         * @brief Same as mem_->set_value_at(uint32_t, uint32_t)
         */
        void set_mem_value(uint32_t addr, uint32_t value);

        /**
         * @brief Same as mem_->get_value_at(uint32_t)
         */
        uint32_t get_mem_value(uint32_t addr) const;

    private:
        /**
         * @brief Pointer to global virtual memory object
         *
         * If the implementation wants to use set_mem_value() or get_mem_value()
         * methods, this should be initialized to point to global virtual memory
         * object.
         */
        Memory* mem_;
};

/**
 * @brief IO Handler for the register with separate "set" and "clear" registers.
 *
 * Implements set/clear/rw addresses for the same register.
 *
 * Writing to "set" address sets the bits in the register that are set in the
 * written value. All other bits are preserved.
 *
 * Writing to "clear" address clears the bits in the register that are *set* in
 * the written value. All other bits are preserved.
 *
 * The main address has usual Read/Write access. All addresses read as a current
 * value for the register.
 */
class RegSetClearStub : public IOHandlerStub {
    public:
        /**
         * @brief Constructor for the handler.
         *
         * @param addr Normal Read/Write access address.
         * @param set_addr Address for "set" access.
         * @param clr_addr Address for "clear" access.
         */
        RegSetClearStub(uint32_t addr, uint32_t set_addr, uint32_t clr_addr) :
            rw_addr_{addr}, set_addr_{set_addr}, clr_addr_{clr_addr} {}

        uint32_t write32(uint32_t addr, uint32_t old_value, uint32_t new_value) override {
            auto write_value = new_value;
            if (addr == set_addr_) {
                write_value = (old_value | new_value);
            } else if (addr == clr_addr_) {
                write_value = (old_value & (~new_value));
            }

            if (addr != rw_addr_) {
                set_mem_value(rw_addr_, write_value);
                set_mem_value(set_addr_, write_value);
                set_mem_value(clr_addr_, write_value);
            }
            return write_value;
        }

        uint32_t read32(uint32_t addr, uint32_t value) override {
            if (addr == set_addr_ || addr == clr_addr_) {
                return get_mem_value(rw_addr_);
            }

            return value;
        }

    protected:
        const uint32_t rw_addr_;
        const uint32_t set_addr_;
        const uint32_t clr_addr_;
};

/**
 * @brief Ignore writes to the handled virtual memory address.
 *
 * If this handler is assigned to a particular address, all writes
 * to that address will be ignored.
 */
class IgnoreWrites : public IOHandlerStub {
    public:
        uint32_t write32(uint32_t addr, uint32_t old_value, uint32_t new_value) override {
            (void)addr;
            (void)new_value;
            return old_value;
        }
};

/**
 * @brief Source reads from a list.
 *
 * Each subsequent read from the handled address will return a value
 * from the top of the list and pop that value.
 *
 * Useful for simulating incoming transmission of some sort for UART,
 * I2C etc. peripherals.
 */
class SourceIOHandler : public IOHandlerStub {
    public:
        uint32_t read32(uint32_t addr, uint32_t value) override {
            (void)addr;
            uint32_t ret = value;
            if (!read_seq_.empty()) {
                ret = *(read_seq_.begin());
                read_seq_.pop_front();
            }

            return ret;
        }

        /**
         * @brief Add value to the back of the read list
         */
        void add_value(uint32_t value) {
            read_seq_.push_back(value);
        }

        /**
         * @brief Get number of values in read sequence.
         */
        size_t get_seq_len() const {
            return read_seq_.size();
        }

    private:
        std::list<uint32_t> read_seq_;
};

/**
 * @brief Write values into a sink.
 *
 * Each subsequent write to the handled address will be stored in a vector,
 * after being cast to type T.
 *
 * This is useful for simulating outgoing transmission of some sort for UART,
 * I2C etc. peripherals.
 */
template <typename T>
class WriteSink : public IOHandlerStub {
    public:
        uint32_t write32(uint32_t addr, uint32_t old_value, uint32_t new_value) override {
            (void)addr;
            (void)old_value;

            sink_.push_back(static_cast<T>(new_value));
            return new_value;
        }

        /**
         * @brief Clear the sink. Removes all data from it.
         */
        void clear() {
            sink_.clear();
        }

        /**
         * @brief Get reference to the data in the sink.
         */
        const std::vector<T>& get_data() const {
            return sink_;
        }

    private:
        std::vector<T> sink_;
};


/**
 * @brief Write One To Clear Handler.
 *
 * Instead of writing the specified value to the register,
 * this handler will clear the bits set in written value, i.e.
 * if the bit is set in the written value, it will be cleared
 * in the value stored at specified address.
 */
class W1CStub : public IOHandlerStub {
    public:
        uint32_t write32(uint32_t addr, uint32_t old_value, uint32_t new_value) override {
            (void)addr;

            return old_value & (~new_value);
        }
};


/**
 * @brief Virtual Memory IO handler for tests.
 *
 * The tests should treat this class as a singleton and use mock::get_global_memory() function
 * to get an instance. Memory class' tests do not treat it as a singleton though, so
 * the rule is not strictly enforced.
 *
 * The methods of this class can be split into two groups: simulation and examination.
 * Simulations methods are the ones that simulate memory IO operations for the firmware.
 * These are readptr(), writeptr(), read32(), write32(), read16(), write16(), read8(), write8().
 *
 * The rest of the methods are used in tests to examine or to configure the state of the virtual
 * memory for testing purposes.
 *
 * The class allows further customization of address' handling. One can specify a handler
 * for a specific address or an address range and all IO operations will be forwarded to that
 * handler. This is the main simulation mechanizm.
 */
class Memory {
    public:
        Memory() = default;
        Memory(const Memory&) = delete;
        enum class Op {
            READ8,
            READ16,
            READ32,
            READPTR,
            WRITE8,
            WRITE16,
            WRITE32,
            WRITEPTR,
        };

        /**
         * @brief MemoryIO operations journal entry.
         *
         * This is a tuple of Op, memory address and value. For read operations
         * the value is what the operation has returned. For write operations
         * it is a new value.
         */
        using JournalEntry = std::tuple<Op, uint32_t, uint32_t>;

        /**
         * @brief List of memio operations.
         */
        using JournalT = std::vector<JournalEntry>;

        /**
         * @brief Get the journal for examination.
         *
         * There should not be any reason to modify the journal,
         * so this return a constant reference.
         */
        const JournalT& get_journal() const;

        /**
         * @brief Reset the Memory object.
         *
         * This clears the journal, all IO handlers and all of the virtual
         * memory contents. The test suite should call this before
         * performing any of the IO operations.
         */
        void reset();

        /**
         * @brief Read pointer from virtual memory address.
         *
         * Called by the simulation.
         *
         * This needs to be previously set by set_ptr_at() or writeptr(), otherwise
         * it would return nullptr.
         */
        void* readptr(uint32_t addr) const;

        /**
         * @brief Read 32 bit value from 32 bit aligned address.
         *
         * Called by the simulation.
         */
        uint32_t read32(uint32_t addr) const;

        /**
         * @brief Read 16 bit value from 16 bit aligned address.
         *
         * Called by the simulation.
         */
        uint16_t read16(uint32_t addr) const;

        /**
         * @brief Read 8 bit value from the address.
         *
         * Called by the simulation.
         */
        uint8_t read8(uint32_t addr) const;

        /**
         * @brief Store the pointer (the address it points to) at virtual memory address.
         *
         * Called by the simulation.
         *
         * Since simulated ARM MCU's are 32bit and most desktop OS (where the tests are run)
         * are 64 bit, storing and reading pointers needs special handling.
         */
        void writeptr(uint32_t addr, void* ptr);

        /**
         * @brief Store 32 bit value at 32 bit aligned virtual memory address.
         *
         * Called by the simulation.
         */
        void write32(uint32_t addr, uint32_t value);

        /**
         * @brief Store 16 bit value at 16 bit aligned virtual memory address.
         *
         * Called by the simulation.
         */
        void write16(uint32_t addr, uint16_t value);

        /**
         * @brief Store 8 bit value in virtual memory address.
         *
         * Called by the simulation.
         */
        void write8(uint32_t addr, uint8_t value);

        /**
         * @brief Store value at virtual memory address.
         *
         * The difference from write32() is that this method bypasses the journal
         * and if there are any IOHandlerStub objects registered for that address,
         * this also bypasses them.
         *
         * This meant to be used by tests.
         */
        void set_value_at(uint32_t addr, uint32_t value);

        /**
         * @brief Get value at virtual memory address.
         *
         * The difference from read32() is that this method bypasses the journal
         * and if there are any IOHandlerStub objects registered for that address,
         * this also bypasses them.
         *
         * This meant to be used by tests.
         */
        uint32_t get_value_at(uint32_t addr) const;

        /**
         * @brief Get value at virtual memory address.
         *
         * The difference from read32() is that this method bypasses the journal
         * and if there are any IOHandlerStub objects registered for that address,
         * this also bypasses them. If the value at the address was not previously set,
         * returns default value.
         *
         * The difference from get_value_at(uint32_t) is that this method does
         * not throw an exception when the value wasn't initialized.
         *
         * This meant to be used by tests.
         */
        uint32_t get_value_at(uint32_t addr, uint32_t default_value) const;

        /**
         * @brief Get pointer stored at address.
         *
         * The method will return nullptr, if the address wasn't initialized.
         */
        void* get_ptr_at(uint32_t addr) const;

        /**
         * @brief Store pointer at virtual memory address.
         */
        void set_ptr_at(uint32_t addr, void* ptr);

        /**
         * @brief Set IOHandlerStub for a single 32bit aligned address.
         *
         * @param[in] addr Virtual memory address. Must be 32 bit aligned.
         * @param[in] io_handler All memory IO operations on the address will
         *  be forwarded to this object. Note, that Memory object does not own the pointer,
         *  which is a primary reason why calling reset() at the beginning of a test is required.
         */
        void set_addr_io_handler(uint32_t addr, IOHandlerStub* io_handler);

        /**
         * @brief Set IOHandlerStub for a range of virtual memory addresses.
         *
         * The range is [range_start; range_end).
         *
         * @param[in] range_start Virtual memory address. Must be 32 bit aligned.
         * @param[in] range_end Virtual memory address. Must be 32 bit aligned.
         * @param[in] io_handler All memory IO operations on the address will
         *  be forwarded to this object. Note, that Memory object does not own the pointer,
         *  which is a primary reason why calling reset() at the beginning of a test is required.
         */
        void set_addr_io_handler(uint32_t range_start, uint32_t range_end, IOHandlerStub* io_handler);

        /**
         * @brief Count the number of operations performed.
         *
         * This basically looks at the journal and counts how many operations
         * are of this type.
         *
         * @param[in] op The operation the caller is interested in.
         */
        unsigned int get_op_count(Op op) const;

        /**
         * @brief Count the number of operations performed.
         *
         * This basically looks at the journal and counts how many operations
         * are of this type and at a given address.
         *
         * @param[in] op The operation the caller is interested in.
         * @param[in] addr The address the caller is interested in.
         */
        unsigned int get_op_count(Op op, uint32_t addr) const;

        /**
         * @brief Print the journal to stdout.
         *
         * This is kind of a last resort for debugging.
         */
        void print_journal() const;

        /**
         * @brief Print virtual memory map to stdout.
         *
         * Only prints cells that were set by the user or simulation, i.e.
         * using set_value_at() or writeXXX() methods.
         * Does not (yet) print stored pointers.
         */
        void print_map() const;

    private:
        void priv_write32(uint32_t addr, uint32_t value);
        uint32_t priv_read32(uint32_t addr) const;

        std::map<uint32_t, uint32_t> mem_map_;
        std::map<uint32_t, void*> mem_ptr_map_;
        // Note, memory mock does not own the pointers.
        // It is the responsibility of the caller to clean them up.
        // Special care needs to be taken in the case of the global
        // memory map object.
        std::map<uint32_t, IOHandlerStub*> addr_handler_map_;
        mutable JournalT journal_;
};

/**
 * @brief Return global Memory object.
 *
 * The tests that user memio mocking should use this function
 * to get the Singleton Memory object.
 */
Memory& get_global_memory();

}  // namespace mock
