#pragma once
#include "tusb.h"

void pzx_keyscan_init();
void __not_in_flash_func(pzx_keyscan_row)();
uint32_t __not_in_flash_func(pzx_keyscan_get_row)(uint32_t ri);
void __not_in_flash_func(pzx_keyscan_get_hid_reports)(hid_keyboard_report_t const **curr, hid_keyboard_report_t const **prev);

