#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"

#include "pzx_keyscan.h"
#include "pzx_keyscan.pio.h"

void pzx_keyscan_init() {
  
    // Choose which PIO instance to use.
    PIO pio = pio1;

    // Our assembled program needs to be loaded into this PIO's instruction
    // memory. This SDK function will find a location (offset) in the
    // instruction memory where there is enough space for our program. We need
    // to remember this location!
    uint offset = pio_add_program(pio, &pzx_keyscan_program);

    // Find a free state machine on our chosen PIO (erroring if there are
    // none). Configure it to run our program, and start it, using the
    // helper function we included in our .pio file.
    uint sm = pio_claim_unused_sm(pio, true);
    
    pzx_keyscan_program_init(pio, sm, offset);
    
}
