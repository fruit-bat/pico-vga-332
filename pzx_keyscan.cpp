#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"

#include "pzx_keyscan.h"
// #include "pzx_keyscan.pio.h"

#define SAMPLES 4 

static uint8_t cp[] = {20, 21, 22, 26, 27, 28};  // Column pins
static uint8_t rp[] = {14, 15, 16, 17, 18, 19};  // Row pins
static uint8_t rs[6][SAMPLES];                   // Oversampled pins
static uint8_t rbd[6];                           // Debounced pins

#define KEY_Q         0x14
#define KEY_W         0x1a
#define KEY_E         0x08
#define KEY_R         0x15
#define KEY_T         0x17
#define KEY_ALT       0xe2
#define KEY_Y         0x1c
#define KEY_U         0x18
#define KEY_I         0x0c
#define KEY_O         0x12
#define KEY_P         0x13
#define KEY_UP        0x52
#define KEY_DOWN      0x51
#define KEY_ESC       0x29
#define KEY_A         0x04
#define KEY_S         0x16
#define KEY_D         0x07
#define KEY_F         0x08
#define KEY_G         0x0a
#define KEY_LEFT      0x50
#define KEY_RIGHT     0x4f
#define KEY_H         0x0b
#define KEY_J         0x0d
#define KEY_K         0x0e
#define KEY_L         0x0f
#define KEY_ENTER     0x28
#define KEY_BACKSPACE 0x2a
#define KEY_Z         0x1d
#define KEY_X         0x1b
#define KEY_C         0x06
#define KEY_V         0x19
#define KEY_SPACE     0x2c
#define KEY_COMMA     0x36
#define KEY_M         0x10
#define KEY_N         0x11
#define KEY_B         0x05

static pzx_keyscan_keyinfo kbits[6][6] = {
  // Row 0
  { {KEY_SPACE, ' ', ' '}, {KEY_COMMA, ',', '<'}, {KEY_M, 'm', 'M'}, {KEY_N, 'n', 'N'}, {KEY_B, 'b', 'B'}, {KEY_DOWN, ' ', ' '} },
  // Row 1
  { {KEY_ENTER, '\n', '\n'}, {KEY_L, 'l', 'L'}, {KEY_K, 'k', 'K'}, {KEY_J, 'j', 'J'}, {KEY_H, 'h', 'H'}, {KEY_LEFT, ' ', ' '} },
  // Row 2
  { {KEY_P, 'p', 'P'}, {KEY_O, 'o', 'O'}, {KEY_I, 'i', 'I'}, {KEY_U, 'u', 'U'}, {KEY_Y, 'y', 'Y'}, {KEY_UP, ' ', ' '} },
  // Row 3
  { {KEY_BACKSPACE, ' ', ' '}, {KEY_Z, 'z', 'Z'}, {KEY_X, 'x', 'X'}, {KEY_C, 'c', 'C'}, {KEY_V, 'v', 'V'}, {KEY_RIGHT, ' ', ' '} },
  // Row 4
  { {KEY_A, 'a', 'A'}, {KEY_S, 's', 'S'}, {KEY_D, 'd', 'D'}, {KEY_F, 'f', 'F'}, {KEY_G, 'g', 'G'}, {KEY_ESC, ' ', ' '} },
  // Row 5
  { {KEY_Q, 'q', 'Q'}, {KEY_W, 'w', 'W'}, {KEY_E, 'e', 'E'}, {KEY_R, 'r', 'R'}, {KEY_T, 't', 'T'}, {KEY_ALT, ' ', ' '} }
};

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
  rbd[ri] = (am | rbd[ri]) & om; 
}

uint32_t __not_in_flash_func(pzx_keyscan_get_row)(uint32_t ri) {
  return rbd[ri] ;
}

void pzx_print_keys(uint32_t ri) {
  uint8_t r = rbd[ri];
  uint32_t i = 0;
  while(r) {
    if (r & 1) {
      pzx_keyscan_keyinfo* ki = &(kbits[ri][i]);
      printf("hid keycode %2.2x, ascii %2.2x, '%c'\n", ki->keycode, ki->normal, ki->normal);
    }
    ++i;
    r >>= 1;
  }
}
