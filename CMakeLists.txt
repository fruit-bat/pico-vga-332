
set(pico_vga_332_common
	${CMAKE_CURRENT_LIST_DIR}/vga_vmode.cpp
	${CMAKE_CURRENT_LIST_DIR}/vga.cpp
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
