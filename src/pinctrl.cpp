#include "pinctrl.hpp"

namespace pinctrl {

int get_pin(int function) {
    if (!board_pin_config) {
        return -1;
    }

    struct entry* c_entry = board_pin_config->entries;
    for (unsigned int i = 0; i < board_pin_config->n_entries; ++i) {
        if (c_entry->function == function) {
            return c_entry->pin;
        }

        ++c_entry;
    }

    return -1;
}

}  // namespace pinctrl
