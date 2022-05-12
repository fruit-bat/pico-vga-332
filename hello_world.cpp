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


#define VGA_RGB_332(r,g,b) ((r<<5)|(g<<2)|b)


static uint8_t zxcolours[16] = {
  VGA_RGB_332(0,0,0), // Black
  VGA_RGB_332(0,0,2), // Blue
  VGA_RGB_332(5,0,0), // Red
  VGA_RGB_332(5,0,2), // Magenta
  VGA_RGB_332(0,5,0), // Green
  VGA_RGB_332(0,5,2), // Cyan
  VGA_RGB_332(5,5,0), // Yellow
  VGA_RGB_332(5,5,2), // White
  VGA_RGB_332(0,0,0), // Black
  VGA_RGB_332(0,0,3), // Bright Blue
  VGA_RGB_332(7,0,0), // Bright Red
  VGA_RGB_332(7,0,3), // Bright Magenta
  VGA_RGB_332(0,7,0), // Bright Green
  VGA_RGB_332(0,7,3), // Bright Cyan
  VGA_RGB_332(7,7,0), // Bright Yellow
  VGA_RGB_332(7,7,3)  // Bright White
};

static uint32_t zx_colour_words[16];

static uint32_t zx_bitbit_masks[4] = {
  0x00000000,
  0xffff0000,
  0x0000ffff,
  0xffffffff
};
static uint32_t zx_invert_masks[] = {
  0x00,
  0xff
};

void init_zx_renderer() {
  for(unsigned int i = 0; i < 16; ++i) {
    uint32_t a = zxcolours[i];
    zx_colour_words[i] = a | (a << 8) | (a << 16) | (a << 24);
    printf("zx col %02x w %08x\n", a, zx_colour_words[i]);
  }
}

static uint8_t screen[6144];
static uint8_t attr[768];

static uint8_t *screenPtr = screen;
static uint8_t *attrPtr = attr;



void __not_in_flash_func(zx_render_line)(uint32_t* buf, uint32_t y, uint32_t frame) {
	const uint8_t borderColor = 1; // TODO fetch from emulator

  const uint32_t bw = zx_colour_words[borderColor];
	
	if (y < 24 || y >= (24+192)) {
    // Screen is 640 bytes
    // Each color word is 4 bytes (and represents 2 pixels
    for (int i = 0; i < 160; ++i) buf[i] = bw;
	}
	else {
    // 640 - (256 * 2) = 128
    // Border edge is 64 bytes wide
    for (int i = 0; i < 16; ++i) *buf++ = bw;
    
    const uint v = y - 24;
    const uint8_t *s = screenPtr + ((v & 0x7) << 8) + ((v & 0x38) << 2) + ((v & 0xc0) << 5);
    const uint8_t *a = attrPtr+((v>>3)<<5);
    const int m = (frame >> 5) & 1;   
     
    for (int i = 0; i < 32; ++i) {
      uint8_t c = *a++; // Fetch the attribute for the character
      uint8_t p = *s++ ^ zx_invert_masks[(c >> 7) & m]; // fetch a byte of pixel data
      uint8_t bci = (c >> 3) & 0xf; // The background colour 
      uint8_t fci = (c & 7) | (bci & 0x8); // The foreground colour index
      uint32_t bcw = zx_colour_words[bci]; // The background colour word
      uint32_t fcw = zx_colour_words[fci]; // The foreground colour word
      uint32_t fgm;
      uint32_t bgm;
      fgm = zx_bitbit_masks[(p >> 6) & 3];
      bgm = ~fgm;
      *buf++ = (fgm & fcw) | (bgm & bcw);     
      fgm = zx_bitbit_masks[(p >> 4) & 3];
      bgm = ~fgm;
      *buf++ = (fgm & fcw) | (bgm & bcw);     
      fgm = zx_bitbit_masks[(p >> 2) & 3];
      bgm = ~fgm;
      *buf++ = (fgm & fcw) | (bgm & bcw);     
      fgm = zx_bitbit_masks[p & 3];
      bgm = ~fgm;
      *buf++ = (fgm & fcw) | (bgm & bcw);           
    }
    
    for (int i = 0; i < 16; ++i) *buf++ = bw;
  }
}

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
    //pcw_prepare_vga332_scanline_80(buf, y, linebuf->frame);
    zx_render_line(buf, y, linebuf->frame);
  }
  __builtin_unreachable();
}

int main(){
  vreg_set_voltage(VREG_VSEL);
  sleep_ms(10);
  vmode = Video(DEV_VGA, RES_HVGA);
  sleep_ms(100);

  //Initialise I/O
  stdio_init_all(); 

  for(unsigned int i = 0; i < sizeof(screen); ++i) {
    screen[i] = (0xff & i);
  }
  for(unsigned int i = 0; i < sizeof(attr); ++i) {
    attr[i] = (0xff & i);
  }
  
  pcw_init_renderer();
  init_zx_renderer();
  

  sleep_ms(10);
  
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
