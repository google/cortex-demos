#pragma once

namespace periph {

namespace {

constexpr uint32_t kPeriphBase = 0x40000000;

}  // namespace

constexpr uint32_t id_to_base(unsigned int p_id) {
    return kPeriphBase + p_id * 0x1000;
}

}  // namespace periph
