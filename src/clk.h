#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int clk_request(int clock_id);
unsigned int clk_get_rate(int clock_id);
unsigned int clk_request_rate(int clock_id, unsigned int rate);

#ifdef __cplusplus
}
#endif
