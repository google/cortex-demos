#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int clk_request(int clock_id);
unsigned int clk_get_rate(int clock_id);

#ifdef __cplusplus
}
#endif
