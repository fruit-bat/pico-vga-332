#pragma once

void pzx_keyscan_init();
void __not_in_flash_func(pzx_keyscan_row)();
uint32_t __not_in_flash_func(pzx_keyscan_get_row)(uint32_t ri);
