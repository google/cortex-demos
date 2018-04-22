#include "clk.h"

#include <stdint.h>

#include "memio.h"
#include "nrf52/clk.h"

#define CLOCK_BASE      (0x40000000)

#define LFCLKSRC        (CLOCK_BASE + 0x518)
#define LFCLKSRC_SRC_MASK       (3)
#define LFCLKSRC_BYPASS       (1 << 16)
#define LFCLKSRC_EXTERNAL       (1 << 17)

#define LFCLKSTAT       (CLOCK_BASE + 0x418)
#define LFCLKSTAT_RUNNING     (1 << 16)

#define TASK(n)     (CLOCK_BASE + ((n) * 4))

enum {
    TASK_HFCLKSTART,
    TASK_HFCLKSTOP,
    TASK_LFCLKSTART,
    TASK_LFCLKSTOP,
    TASK_CAL,
    TASK_CTSTART,
    TASK_CTSTOP,
};

enum {
    LFCLK_SRC_RC,
    LFCLK_SRC_XTAL,
    LFCLK_SRC_SYNTH,

    LFCLK_SRC_LAST,
};

static void start_lfclk_sync(void) {
    raw_write32(TASK(TASK_LFCLKSTART), 1);
    while (!(raw_read32(LFCLKSTAT) & LFCLKSTAT_RUNNING));
}

int clk_request(int clock_id) {
    int ret = -1;
    switch (clock_id) {
        case NRF52_LFCLK_RC:
            raw_write32(LFCLKSRC, LFCLK_SRC_RC);
            start_lfclk_sync();
            ret = 0;
            break;
        case NRF52_LFCLK_XTAL:
            raw_write32(LFCLKSRC, LFCLK_SRC_XTAL);
            start_lfclk_sync();
            ret = 0;
            break;
    }

    return ret;
}
