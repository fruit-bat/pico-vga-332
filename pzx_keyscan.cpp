#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"

#include "pzx_keyscan.h"
// #include "pzx_keyscan.pio.h"

#define SAMPLES 4

static uint8_t cp[] = {20, 21, 22, 26, 27, 28};
static uint8_t rp[] = {14, 15, 16, 17, 18, 19};
static uint8_t rs[6][SAMPLES];

void pzx_keyscan_init() {
  
    for(int i = 0; i < 6; ++i) {
      gpio_init(rp[i]);
      gpio_set_dir(rp[i], GPIO_IN);    
      gpio_disable_pulls(rp[i]);
   }  
    
   for(int i = 0; i < 6; ++i) {
      gpio_init(cp[i]);
      gpio_set_dir(cp[i], GPIO_IN);    
      gpio_pull_up(cp[i]);
   }
   
   uint32_t row = rp[0];
   gpio_set_dir(row, GPIO_OUT);
   gpio_put(row, 0);
}

void __not_in_flash_func(pzx_keyscan_row)() {
  static uint32_t ri = 0;
  static uint32_t si = 0;
  uint32_t a = gpio_get_all() >> 20;
  uint32_t r = (a & 7) | ((a >> 3) & (7 << 3));
  rs[ri][si] = (uint8_t)r;
  uint32_t row;
  row = rp[ri];
  gpio_set_dir(row, GPIO_IN);
  gpio_disable_pulls(row);
  if (++ri >= 6) {
    ri = 0;
    if (++si >= SAMPLES) si = 0;
  }
  row = rp[ri];
  gpio_set_dir(row, GPIO_OUT);
  gpio_put(row, 0);
}

uint32_t __not_in_flash_func(pzx_keyscan_get_row)(uint32_t ri) {
  uint32_t a = 0;
  for(int si = 0; si < SAMPLES; ++si) {
    a |= rs[ri][si];
  }
  return a;
}
