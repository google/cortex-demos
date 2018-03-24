#pragma once

#include <stdint.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void syscontrol_relocate_vt(ubase_t new_addr, unsigned num_vectors);

#ifdef __cplusplus
}  /* extern "C" */
#endif
