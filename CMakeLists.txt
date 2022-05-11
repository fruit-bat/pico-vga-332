cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico-hello-world C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

include(../PicoDVI/software/apps/pico-dvi-menu/CMakeLists.txt)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )
        
add_executable(hello_world
    hello_world.c
    vga_vmode.cpp
    vga.cpp
)

pico_generate_pio_header(hello_world 
  ${CMAKE_CURRENT_LIST_DIR}/picovga.pio
)

target_link_libraries(hello_world 
  pico_stdlib 
  pico_multicore
  hardware_dma
  hardware_irq
  hardware_pio
  pico_vga_menu
)

pico_enable_stdio_usb(hello_world 1)
pico_enable_stdio_uart(hello_world 0)

pico_add_extra_outputs(hello_world)
