#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void syscontrol_relocate_vt(uint32_t new_addr, unsigned num_vectors);

#ifdef __cplusplus
}  /* extern "C" */
#endif
