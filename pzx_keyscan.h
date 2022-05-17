#pragma once

typedef struct  {
  uint8_t keycode;
  char normal;
  char shifted;
} pzx_keyscan_keyinfo;

void pzx_keyscan_init();
void __not_in_flash_func(pzx_keyscan_row)();
uint32_t __not_in_flash_func(pzx_keyscan_get_row)(uint32_t ri);
void pzx_print_keys(uint32_t ri);
