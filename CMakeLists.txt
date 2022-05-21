# #[[ 

set(pico_vga_332_common
	${CMAKE_CURRENT_LIST_DIR}/vga_vmode.cpp
	${CMAKE_CURRENT_LIST_DIR}/vga.cpp
	${CMAKE_CURRENT_LIST_DIR}/pzx_prepare_vga332_scanline.cpp
	${CMAKE_CURRENT_LIST_DIR}/pzx_keyscan.cpp
)

add_library(pico_vga_332 INTERFACE)

target_sources(pico_vga_332 INTERFACE
	${pico_vga_332_common}
)

target_include_directories(pico_vga_332 INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}
)

pico_generate_pio_header(pico_vga_332 
  ${CMAKE_CURRENT_LIST_DIR}/picovga.pio
)

# ]]
#[[

cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico-hello-world C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_definitions(
  -DPCS_COLS=80
  -DPCS_ROWS=30
  
)

add_subdirectory(/home/pi/pico/PicoDVI/software/apps/pico-emu-utils/ "${CMAKE_CURRENT_BINARY_DIR}/pico-emu-utils")
add_subdirectory(/home/pi/pico/PicoDVI/software/apps/pico-dvi-menu/ "${CMAKE_CURRENT_BINARY_DIR}/pico-dvi-menu")
add_subdirectory(/home/pi/pico/PicoDVI/software/apps/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI "${CMAKE_CURRENT_BINARY_DIR}/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI")

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )


add_executable(hello_world
    hello_world.cpp
    vga_vmode.cpp
    vga.cpp
    pzx_prepare_vga332_scanline.cpp
    pzx_keyscan.cpp
    hw_config.c
)

target_compile_definitions(hello_world PRIVATE PICO_MALLOC_PANIC=1)

pico_generate_pio_header(hello_world 
  ${CMAKE_CURRENT_LIST_DIR}/picovga.pio
)

target_link_libraries(hello_world
  pico_stdlib 
  pico_multicore
	pico_util  
  hardware_dma
  hardware_irq
  hardware_pio
  pico_vga_menu
  pico_emu_utils
  FatFs_SPI
)

pico_enable_stdio_usb(hello_world 1)
pico_enable_stdio_uart(hello_world 0)

pico_add_extra_outputs(hello_world)

]]
