#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int clk_request(int clk_id);
int clk_request_option(int clk_id, int option);
unsigned int clk_get_rate(int clk_id);
unsigned int clk_request_rate(int clk_id, unsigned int rate);

#ifdef __cplusplus
}
#endif
