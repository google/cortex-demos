/*******************************************************************************
    Copyright 2018 Google LLC

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

#include "clk.h"

#include <stdint.h>
#include <stdbool.h>

#include "cutils.h"
#include "memio.h"
#include "sam4s/clk.h"

#define PMC_BASE        (0x400e0400)
#define PMC_PCER0       (PMC_BASE + 0x10)
#define PMC_PCDR0       (PMC_BASE + 0x14)
#define PMC_PCSR0       (PMC_BASE + 0x18)
#define CKGR_MOR        (PMC_BASE + 0x20)
#define CKGR_MCFR        (PMC_BASE + 0x24)
#define CKGR_PLLAR        (PMC_BASE + 0x28)
#define CKGR_PLLBR        (PMC_BASE + 0x2c)
#define PMC_MCKR        (PMC_BASE + 0x30)
#define PMC_SR        (PMC_BASE + 0x68)
#define PMC_PCER1       (PMC_BASE + 0x100)
#define PMC_PCDR1       (PMC_BASE + 0x104)
#define PMC_PCSR1       (PMC_BASE + 0x108)

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

#define CKGR_PLLAR_ONE      (1 << 29)
/* TODO: get MASK/SHIFT thing in order.
 * Makes more sense to define mask 0 based
 * and then also define shift
 */
#define PLL_DIV_MASK    (0xff)
#define PLL_DIV_SHIFT   (0)
#define PLL_MUL_MASK    (0x7ff)
#define PLL_MUL_SHIFT    (16)

#define PMC_MCKR_CSS_MASK   (3)
#define PMC_MCKR_CSS_SHIFT   (0)
#define PMC_MCKR_CSS_SLOW_CLK   (0)
#define PMC_MCKR_CSS_MAIN_CLK   (1)
#define PMC_MCKR_CSS_PLLA_CLK   (2)
#define PMC_MCKR_CSS_PLLB_CLK   (3)

#define PMC_MCKR_PRES_MASK  (7)
#define PMC_MCKR_PRES_SHIFT  (4)

#define PMC_MCKR_PLLADIV2   (1 << 12)
#define PMC_MCKR_PLLBDIV2   (1 << 13)

#define PMC_SR_MOSCXTS      (1 << 0)
#define PMC_SR_LOCKA    (1 << 1)
#define PMC_SR_LOCKB    (1 << 2)
#define PMC_SR_MCKRDY   (1 << 3)
#define PMC_SR_MOSCSELS      (1 << 16)

#define SUPC_CR_XTALSEL     (1 << 3)
#define SUPC_CR_KEY     (0xa5 << 24)

#define SUPC_SR_OSCSEL      (1 << 7)

/* ~15ms for good measure.
 * Datasheet specifies 14.5 ms startup time for 3MHz OSC,
 * which is the maximum.
 */
#define CRYSTAL_STARTUP_TICKS (62 << 8)

#define PLL_STARTUP_TICKS (5 << 8)

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

void sam4s_set_crystal_frequency(unsigned int value) {
    crystal_rate = value;
}

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
    if (clk_id >= SAM4S_CLK_PIDCK(8) && clk_id < SAM4S_CLK_PIDCK(32)) {
        raw_write32(PMC_PCER0, (1 << (clk_id - SAM4S_CLK_PIDCK(0))));
        return 0;
    } else if (clk_id >= SAM4S_CLK_PIDCK(32) && clk_id < SAM4S_CLK_PIDCK(35)) {
        raw_write32(PMC_PCER1, (1 << (clk_id - SAM4S_CLK_PIDCK(32))));
        return 0;
    }

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

static inline unsigned _get_pll_rate(uint32_t pllr, uint32_t div2_flag) {
    const unsigned mul = ((pllr >> PLL_MUL_SHIFT) & PLL_MUL_MASK);
    const unsigned div = ((pllr >> PLL_DIV_SHIFT) & PLL_DIV_MASK);

    if (div == 0 || mul == 0) {
        return 0;
    }

    const unsigned base_rate = clk_get_rate(SAM4S_CLK_MAINCK);

    unsigned div_in_rate = (base_rate * (mul + 1)) / div;
    return div2_flag ? div_in_rate / 2 : div_in_rate;
}

static inline unsigned int _get_mck_rate(void) {
    const uint32_t mckr = raw_read32(PMC_MCKR);
    unsigned mckr_src_rate = 0;
    switch (mckr & PMC_MCKR_CSS_MASK) {
    case PMC_MCKR_CSS_SLOW_CLK:
        mckr_src_rate = clk_get_rate(SAM4S_CLK_SLCK);
        break;
    case PMC_MCKR_CSS_MAIN_CLK:
        mckr_src_rate = clk_get_rate(SAM4S_CLK_MAINCK);
        break;
    case PMC_MCKR_CSS_PLLA_CLK:
        mckr_src_rate = clk_get_rate(SAM4S_CLK_PLLACK);
        break;
    case PMC_MCKR_CSS_PLLB_CLK:
        mckr_src_rate = clk_get_rate(SAM4S_CLK_PLLBCK);
        break;
    }

    uint32_t pres = (mckr >> PMC_MCKR_PRES_SHIFT) & PMC_MCKR_PRES_MASK;
    if (pres < 7) {
        mckr_src_rate /= (1 << pres);
    } else {
        mckr_src_rate /= 3;
    }

    return mckr_src_rate;
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
        ret = crystal_rate;
        break;
    case SAM4S_CLK_PLLACK:
        ret = _get_pll_rate(raw_read32(CKGR_PLLAR), raw_read32(PMC_MCKR) & PMC_MCKR_PLLADIV2);
        break;
    case SAM4S_CLK_PLLBCK:
        ret = _get_pll_rate(raw_read32(CKGR_PLLBR), raw_read32(PMC_MCKR) & PMC_MCKR_PLLBDIV2);
        break;
    case SAM4S_CLK_PCK:
    case SAM4S_CLK_FCLK:
    case SAM4S_CLK_HCLK:
    case SAM4S_CLK_MCK:
        ret = _get_mck_rate();
        break;
    default:
        /* FIXME: This returns incorrect value for first 8 clocks
         * Do they matter?
         */
        if (clk_id >= SAM4S_CLK_PIDCK(0) && clk_id < SAM4S_CLK_PIDCK(32)) {
            if (raw_read32(PMC_PCSR0) & (1 << (clk_id - SAM4S_CLK_PIDCK(0)))) {
                ret = _get_mck_rate();
            }
        } else if (clk_id >= SAM4S_CLK_PIDCK(32) && clk_id < SAM4S_CLK_PIDCK(35)) {
            if (raw_read32(PMC_PCSR1) & (1 << (clk_id - SAM4S_CLK_PIDCK(32)))) {
                ret = _get_mck_rate();
            }
        }
    }

    return ret;
}

static inline bool _rate_in_range(unsigned int rate) {
    const unsigned MHz = 1000 * 1000;
    return (rate <= 240 * MHz && rate >= 80 * MHz);
}

static inline unsigned int _configure_pll(uint32_t reg_addr, unsigned int rate, uint32_t flags) {
    if (!_rate_in_range(rate)) {
        /* Unsupported rate */
        return 0;
    }

    unsigned int base_rate = clk_get_rate(SAM4S_CLK_MAINCK);
    /* Brute force the best configuration */
    unsigned int best_rate = 0;
    unsigned int best_mul = 0;
    unsigned int best_div = 1;
    for (unsigned int mul = 7; mul <= 62; ++mul) {
        unsigned int req_div = 1 + (base_rate * (mul + 1) - 1) / rate;
        if (req_div == 0 || req_div > 255) {
            continue;
        }

        unsigned int this_rate = (base_rate * (mul + 1)) / req_div;
        if (this_rate == rate) {
            best_rate = this_rate;
            best_mul = mul;
            best_div = req_div;
            break;
        } else if (this_rate > rate || !_rate_in_range(this_rate)) {
            continue;
        }

        if (rate - this_rate < rate - best_rate) {
            best_rate = this_rate;
            best_mul = mul;
            best_div = req_div;
        }
    }

    if (best_rate > 0) {
        raw_write32(reg_addr, (best_div | PLL_STARTUP_TICKS | (best_mul << PLL_MUL_SHIFT) | flags));
        return best_rate;
    }

    return 0;
}

unsigned int clk_request_rate(int clk_id, unsigned int rate) {
    unsigned int ret = 0;

    switch (clk_id) {
    case SAM4S_CLK_PLLACK:
        ret = _configure_pll(CKGR_PLLAR, rate, CKGR_PLLAR_ONE);
        if (ret > 0) {
            wait_mask_le32(PMC_SR, PMC_SR_LOCKA);
        }
        break;
    case SAM4S_CLK_PLLBCK:
        ret = _configure_pll(CKGR_PLLBR, rate, 0);
        if (ret > 0) {
            wait_mask_le32(PMC_SR, PMC_SR_LOCKB);
        }
        break;
    default:
        ret = clk_get_rate(clk_id);
        break;
    }

    return ret;
}

static inline int _set_mck_option(int option) {
    int ret = -1;
    unsigned parent_rate = 0;
    uint32_t css = PMC_MCKR_CSS_MAIN_CLK;
    switch (option) {
    case SAM4S_CLK_PLLACK:
        parent_rate = clk_get_rate(SAM4S_CLK_PLLACK);
        css = PMC_MCKR_CSS_PLLA_CLK;
        break;
    case SAM4S_CLK_PLLBCK:
        parent_rate = clk_get_rate(SAM4S_CLK_PLLBCK);
        css = PMC_MCKR_CSS_PLLB_CLK;
        break;
    }

    if (parent_rate) {
        /* We aren't using the prescaler, so just set CSS and
         * wait for clock to be ready.
         * If the prescaler is used, it needs to be configured before
         * CSS in similar manner.
         */
        uint32_t pmc_mckr = raw_read32(PMC_MCKR);
        pmc_mckr &= ~(PMC_MCKR_CSS_MASK << PMC_MCKR_CSS_SHIFT);
        pmc_mckr |= css;
        raw_write32(PMC_MCKR, pmc_mckr);
        wait_mask_le32(PMC_SR, PMC_SR_MCKRDY);
        ret = 0;
    }

    return ret;
}

int clk_request_option(int clk_id, int option) {
    int ret = -1;
    switch (clk_id) {
    case SAM4S_CLK_MCK:
        ret = _set_mck_option(option);
        break;
    }

    return ret;
}
