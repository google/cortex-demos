#include "clk.h"

#include <stdint.h>

#include "memio.h"
#include "sam4s/clk.h"

#define PMC_BASE        (0x400e0400)
#define CKGR_MOR        (PMC_BASE + 0x20)
#define PMC_SR        (PMC_BASE + 0x68)

#define CKGR_MOR_MOSCXTEN   (1 << 0)
#define CKGR_MOR_MOSCRCEN   (1 << 3)
#define CKGR_MOR_KEY    (0x37 << 16)
#define CKGR_MOR_MOSCXTST_MASK    (0xff << 8)
#define CKGR_MOR_MOSCSEL   (1 << 24)

#define PMC_SR_MOSCXTS      (1 << 0)
#define PMC_SR_MOSCSELS      (1 << 16)

#define CRYSTAL_STARTUP_TICKS (20 << 8)

#ifndef configASSERT
#define configASSERT(...)
#endif

#ifndef clkASSERT
#define clkASSERT   configASSERT
#endif


static int _start_main_crystal(void) {
    uint32_t current_value = raw_read32(CKGR_MOR);
    current_value &= ~CKGR_MOR_MOSCXTST_MASK;
    raw_write32(CKGR_MOR,
            (current_value | CKGR_MOR_KEY | CKGR_MOR_MOSCXTEN | CRYSTAL_STARTUP_TICKS));

    wait_mask_le32(PMC_SR, PMC_SR_MOSCXTS);
    return 0;
}

static int _switch_to_main_crystal(void) {
    uint32_t current_value = raw_read32(CKGR_MOR);
    current_value &= ~(CKGR_MOR_MOSCXTST_MASK | CKGR_MOR_MOSCRCEN);

    clkASSERT(current_value & CKGR_MOR_MOSCXTEN);

    raw_write32(CKGR_MOR,
            (current_value | CKGR_MOR_KEY | CKGR_MOR_MOSCSEL));

    wait_mask_le32(PMC_SR, PMC_SR_MOSCSELS);
    return 0;
}

int clk_request(int clk_id) {
    int ret = 0;
    switch (clk_id) {
        case SAM4S_CLK_HF_CRYSTAL:
            ret = _start_main_crystal();
            break;
        case SAM4S_CLK_MAINCK:
            ret = _switch_to_main_crystal();
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}
