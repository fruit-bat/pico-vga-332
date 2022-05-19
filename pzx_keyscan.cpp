#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "class/hid/hid.h"
#include "pzx_keyscan.h"

#define SAMPLES 4 

static uint8_t cp[] = {20, 21, 22, 26, 27, 28};  // Column pins
static uint8_t rp[] = {14, 15, 16, 17, 18, 19};  // Row pins
static uint8_t rs[6][SAMPLES];                   // Oversampled pins
static uint8_t rdb[6];                           // Debounced pins
static hid_keyboard_report_t hr[2];              // Current and previous hid report
static uint8_t hri = 0;                          // Currenct hid report index
static uint8_t kbi = 0;
static uint8_t kbits[2][6][6] = { 
  {
    // Row 0
    { HID_KEY_SPACE, HID_KEY_COMMA, HID_KEY_M, HID_KEY_N, HID_KEY_B, HID_KEY_ARROW_DOWN },
    // Row 1
    { HID_KEY_ENTER, HID_KEY_L, HID_KEY_K, HID_KEY_J, HID_KEY_H, HID_KEY_ARROW_LEFT },
    // Row 2
    { HID_KEY_P, HID_KEY_O, HID_KEY_I, HID_KEY_U, HID_KEY_Y, HID_KEY_ARROW_UP },
    // Row 3
    { HID_KEY_BACKSPACE, HID_KEY_Z, HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_ARROW_RIGHT },
    // Row 4
    { HID_KEY_A, HID_KEY_S, HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_ESCAPE },
    // Row 5
    { HID_KEY_Q, HID_KEY_W, HID_KEY_E, HID_KEY_R, HID_KEY_T, HID_KEY_ALT_LEFT }
  },
  {
    // Row 0
    { HID_KEY_SPACE, HID_KEY_SEMICOLON, HID_KEY_M, HID_KEY_N, HID_KEY_B, HID_KEY_ARROW_DOWN },
    // Row 1
    { HID_KEY_ENTER, HID_KEY_MINUS, HID_KEY_K, HID_KEY_J, HID_KEY_H, HID_KEY_ARROW_LEFT },
    // Row 2
    { HID_KEY_0, HID_KEY_9, HID_KEY_8, HID_KEY_7, HID_KEY_6, HID_KEY_ARROW_UP },
    // Row 3
    { HID_KEY_EQUAL, HID_KEY_Z, HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_ARROW_RIGHT },
    // Row 4
    { HID_KEY_A, HID_KEY_S, HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_CAPS_LOCK },
    // Row 5
    { HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_ALT_LEFT }
  }
};

#define KEY_ALT_ROW 5
#define KEY_ALT_BIT 0x20
#define KEY_UP_ROW 2
#define KEY_UP_BIT 0x20
#define KEY_DOWN_ROW 0
#define KEY_DOWN_BIT 0x20
#define KEY_LEFT_ROW 1
#define KEY_LEFT_BIT 0x20
#define KEY_RIGHT_ROW 3
#define KEY_RIGHT_BIT 0x20


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
  uint32_t a = ~(gpio_get_all() >> 20);
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
  
  uint32_t om = 0;
  uint32_t am = 0x3f;
  for(int si = 0; si < SAMPLES; ++si) {
    uint8_t s = rs[ri][si];
    om |= s; // bits 0 if no samples have the button pressed
    am &= s; // bits 1 if all samples have the button pressed
  }
  // only change key state if all samples on or off
  rdb[ri] = (am | rdb[ri]) & om; 
}

uint32_t __not_in_flash_func(pzx_keyscan_get_row)(uint32_t ri) {
  return rdb[ri] ;
}

void __not_in_flash_func(pzx_keyscan_get_hid_reports)(hid_keyboard_report_t const **curr, hid_keyboard_report_t const **prev) {

  static uint8_t modifier = 0;
  
  if (rdb[KEY_ALT_ROW] & KEY_ALT_BIT) {
    if (rdb[KEY_UP_ROW] & KEY_UP_BIT) {
      // Shift on
      modifier |= 2;
    }
    else if (rdb[KEY_DOWN_ROW] & KEY_DOWN_BIT) {
      // Shift off
      modifier &= ~2;
    }
    if (rdb[KEY_RIGHT_ROW] & KEY_RIGHT_BIT) {
      // Alt keyboard on
      kbi = 1;
    }
    else if (rdb[KEY_LEFT_ROW] & KEY_LEFT_BIT) {
      // Alt keyboard off
      kbi = 0;
    }
    rdb[KEY_UP_ROW] &= ~KEY_UP_BIT;
    rdb[KEY_DOWN_ROW] &= ~KEY_DOWN_BIT;
    rdb[KEY_RIGHT_ROW] &= ~KEY_RIGHT_BIT;
    rdb[KEY_LEFT_ROW] &= ~KEY_LEFT_BIT;
  }
  
  // Build the current hid report
  uint32_t ki = 0;
  hid_keyboard_report_t *chr = &hr[hri & 1];
  chr->modifier = modifier;
  for(int ri = 0; ri < 6; ++ri) {
    uint8_t r = rdb[ri];
    uint32_t ci = 0;
    while(r) {
      if (r & 1) {
        if (ki >= sizeof(chr->keycode)) {
          // overflow
          ki = 0;
          while(ki < sizeof(chr->keycode)) chr->keycode[ki++] = 1;
          break;  
        }
        uint8_t kc = kbits[kbi & 1][ri][ci];
        chr->keycode[ki++] = kc;
      }
      ++ci;
      r >>= 1;
    }
  }
  while(ki < sizeof(chr->keycode)) chr->keycode[ki++] = 0;
  *curr = chr;
  hri++;
  *prev = &hr[hri & 1];
}

