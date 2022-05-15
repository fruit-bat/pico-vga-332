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
      gpio_init(cp[i]);
      gpio_set_dir(cp[i], GPIO_IN);    
      gpio_disable_pulls(rp[i]);
   }  
    
   for(int i = 0; i < 6; ++i) {
      gpio_init(rp[i]);
      gpio_set_dir(rp[i], GPIO_IN);    
      gpio_pull_up(rp[i]);
   }
   
   uint32_t col = cp[0];
   gpio_set_dir(col, GPIO_OUT);
   gpio_put(col, 0);    
  
}

void __not_in_flash_func(pzx_keyscan_col)() {
  static uint32_t ci = 0;
  static uint32_t si = 0;
  uint32_t r = (gpio_get_all() >> 14) & 63;
  rs[ci][si] = (uint8_t)r;
  uint32_t col;
  col = cp[ci];
  gpio_set_dir(col, GPIO_IN);
  gpio_disable_pulls(col);
  if (ci++ >= 6) {
    ci = 0;
    if (si++ >= SAMPLES) si = 0; // TODO mask>
  }
  col = cp[ci];
  gpio_set_dir(col, GPIO_OUT);
  gpio_put(col, 0);
}
