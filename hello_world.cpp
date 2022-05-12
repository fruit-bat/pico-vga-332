#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/vreg.h"
#include "vga.h"

#define VREG_VSEL VREG_VOLTAGE_1_20
#include "PicoCharRendererVga.h"
#include "PicoWinHidKeyboard.h"
#include "PicoDisplay.h"
#include "PicoPen.h"

struct semaphore dvi_start_sem;
static const sVmode* vmode = NULL;

/*
// __attribute__((aligned(4))) uint16_t charbuffer[PCS_COLS * PCS_ROWS];

#include "VGA_font8x8.h"

static u32 nibblebits[16];


void __not_in_flash_func(VgaRenderLine6)(uint32_t* buf, int y, uint32_t frames) {
  const uint16_t m = (frames >> 5) & 1;
  int np = 0;
  int li = __mul_instruction(y >> 3, PCS_COLS);
  int yb = y & 7;
  // Assume even number of character columns.
  // charbuffer must be 32bit word aligned.
  uint32_t *cl = (uint32_t *)(charbuffer + li);
  for (int x = 0; x < (PCS_COLS >> 1); ++x) {
    uint32_t ch = *cl++;
    uint16_t z;
    int b;
    z = ((ch >> 8) ^ ((ch >> 9) & m)) & 1; 
    b = font8x8[ch & 0xff][yb] ^ __mul_instruction(z, 0xff);
    buf[np++] = nibblebits[b & 0xf];
    buf[np++] = nibblebits[b >> 4];
    ch = ch >> 16;
    z = ((ch >> 8) ^ ((ch >> 9) & m)) & 1; 
    b = font8x8[ch & 0xff][yb] ^ __mul_instruction(z, 0xff);
    buf[np++] = nibblebits[b & 0xf];
    buf[np++] = nibblebits[b >> 4];
  }
}

void init() {
  for(int i = 0; i < 16; ++i) {
    u32 a = 0;
    for(int j = 0; j < 4; ++j) {
      if (i & (1 << j)) a |= (0xff << (j << 3));
    }
    nibblebits[i] = a;
  }


  for (int x = 0; x < PCS_COLS; ++x) {
    for (int y = 0; y < 60; ++y) {
      int i = x + (y * PCS_COLS);
      int j = (x & 3) << 8;
      charbuffer[i] = (i & 0x7f) | j;
    }
  } 
}



*/

void __not_in_flash_func(core1_main)() {
  sem_acquire_blocking(&dvi_start_sem);
  printf("Core 1 running...\n");

  // TODO fetch the resolution from the mode ?
  VgaInit(vmode,640,480);

  while (1) {

    VgaLineBuf *linebuf = get_vga_line();
    uint32_t* buf = (uint32_t*)&(linebuf->line);
    uint32_t y = linebuf->row;
    pcw_prepare_vga332_scanline_80(buf, y, linebuf->frame);

  }
  __builtin_unreachable();
}

int main(){
  vreg_set_voltage(VREG_VSEL);
  sleep_ms(10);

  //Initialise I/O
  stdio_init_all(); 
  pcw_init_renderer();
  
  vmode = Video(DEV_VGA, RES_HVGA);
  
  printf("Core 0 VCO %d\n", Vmode.vco);

  printf("Core 1 start\n");
  sem_init(&dvi_start_sem, 0, 1);
  multicore_launch_core1(core1_main);
    
  PicoWin picoRootWin(10, 10, 60, 10);
  PicoDisplay picoDisplay(pcw_screen(), &picoRootWin);
  picoRootWin.onPaint([=](PicoPen *pen){
    pen->printAtF(24, 4, false,"Hello World!");
  });
  
  printf("Core 0 VCO %d\n", Vmode.vco);

  sem_release(&dvi_start_sem);

    //Main Loop 
    while(1){

        printf("Hello ");
        sleep_ms(1000); // 0.5s delay

        picoDisplay.refresh();

        printf("world!\n");
        sleep_ms(1000); // 0.5s delay
    }
}
