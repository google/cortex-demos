#pragma once

#include <stdint.h>

namespace pinctrl {

struct entry {
    int function;
    int pin;
};

struct config {
    unsigned int n_entries;
    struct entry* entries;
};

extern struct config* board_pin_config;

// Note, that -1 should not be used as a pin number,
// as this is a way to indicate errors.

int get_pin(int function);

}  // pinctrl
