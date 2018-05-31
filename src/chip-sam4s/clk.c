#include "clk.h"

#include <stdint.h>

#include "memio.h"
#include "sam4s/clk.h"

#define PMC_BASE        (0x400e0400)
#define CKGR_MOR        (PMC_BASE + 0x20)
#define CKGR_MCFR        (PMC_BASE + 0x24)
#define PMC_SR        (PMC_BASE + 0x68)

#define SUPC_BASE       (0x400e1410)
#define SUPC_CR     (SUPC_BASE)
#define SUPC_SR     (SUPC_BASE + 0x14)

#define CKGR_MOR_MOSCXTEN   (1 << 0)
#define CKGR_MOR_MOSCRCEN   (1 << 3)
#define CKGR_MOR_KEY    (0x37 << 16)
#define CKGR_MOR_MOSCXTST_MASK    (0xff << 8)
#define CKGR_MOR_MOSCSEL   (1 << 24)
#define CKGR_MOR_MOSCRCF_MASK    (7 << 4)
#define CKGR_MOR_MOSCRCF_SHIFT    (4)

#define CKGR_MCFR_MAINFRDY  (1 << 16)
#define CKGR_MCFR_MAINF_MASK  (0xffff)

#define PMC_SR_MOSCXTS      (1 << 0)
#define PMC_SR_MOSCSELS      (1 << 16)

#define SUPC_CR_XTALSEL     (1 << 3)
#define SUPC_CR_KEY     (0xa5 << 24)

#define SUPC_SR_OSCSEL      (1 << 7)

#define CRYSTAL_STARTUP_TICKS (20 << 8)

#define XTAL_RC_RATE    (32000)
#define XTAL_EXT_RATE   (32768)
#define FASTRC_BASE_RATE        (4 * 1000 * 1000)

#ifndef configASSERT
#define configASSERT(...)
#endif

#ifndef clkASSERT
#define clkASSERT   configASSERT
#endif

static unsigned int crystal_rate = 0;

static int _start_main_crystal(void) {
    uint32_t current_value = raw_read32(CKGR_MOR);
    current_value &= ~CKGR_MOR_MOSCXTST_MASK;
    raw_write32(CKGR_MOR,
            (current_value | CKGR_MOR_KEY | CKGR_MOR_MOSCXTEN | CRYSTAL_STARTUP_TICKS));

    wait_mask_le32(PMC_SR, PMC_SR_MOSCXTS);
    return 0;
}

static inline unsigned _get_slck_rate(void) {
    uint32_t supc_sr = raw_read32(SUPC_SR);
    if (supc_sr & SUPC_SR_OSCSEL) {
        return XTAL_EXT_RATE;
    } else {
        return XTAL_RC_RATE;
    }
}

static int _switch_to_main_crystal(void) {
    uint32_t current_value = raw_read32(CKGR_MOR);
    current_value &= ~(CKGR_MOR_MOSCXTST_MASK | CKGR_MOR_MOSCRCEN);

    clkASSERT(current_value & CKGR_MOR_MOSCXTEN);

    raw_write32(CKGR_MOR,
            (current_value | CKGR_MOR_KEY | CKGR_MOR_MOSCSEL));

    wait_mask_le32(PMC_SR, PMC_SR_MOSCSELS);
    wait_mask_le32(CKGR_MCFR, CKGR_MCFR_MAINFRDY);
    crystal_rate = (raw_read32(CKGR_MCFR) & CKGR_MCFR_MAINF_MASK) * (_get_slck_rate() / 16);
    return 0;
}

static int _switch_to_slow_ext_crystal(void) {
    raw_write32(SUPC_CR, SUPC_CR_KEY | SUPC_CR_XTALSEL);
    wait_mask_le32(SUPC_SR, SUPC_SR_OSCSEL);

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
        case SAM4S_CLK_XTAL_EXT:
            ret = _switch_to_slow_ext_crystal();
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

static inline unsigned _get_fastrc_rate(void) {
    uint32_t ckgr_mor = raw_read32(CKGR_MOR);
    if (ckgr_mor & CKGR_MOR_MOSCRCEN) {
        const unsigned mul = (ckgr_mor & CKGR_MOR_MOSCRCF_MASK) >> CKGR_MOR_MOSCRCF_SHIFT;
        return FASTRC_BASE_RATE * (mul + 1);
    } else {
        return 0;
    }
}

static inline unsigned _get_mainck_rate(void) {
    uint32_t ckgr_mor = raw_read32(CKGR_MOR);
    if (ckgr_mor & CKGR_MOR_MOSCSEL) {
        return crystal_rate;
    }

    return _get_fastrc_rate();
}

unsigned int clk_get_rate(int clk_id) {
    unsigned int ret = 0;
    switch (clk_id) {
        case SAM4S_CLK_XTAL_RC:
            ret = XTAL_RC_RATE;
            break;
        case SAM4S_CLK_XTAL_EXT:
            ret = XTAL_EXT_RATE;
            break;
        case SAM4S_CLK_SLCK:
            ret = _get_slck_rate();
            break;
        case SAM4S_CLK_FAST_RC:
            ret = _get_fastrc_rate();
            break;
        case SAM4S_CLK_MAINCK:
            ret = _get_mainck_rate();
            break;
        case SAM4S_CLK_HF_CRYSTAL:
            return crystal_rate;
            break;
    }

    return ret;
}
