#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "switches.pio.h"

int main() {
    stdio_init_all();

    //bitmask for inputs, 31->0
    //GPIO 31-29 are not available, and 25 is onboard led
    //bits indicated should always be 0
    //                     vvv    v
    //uint32_t bitmask = 0b00000000000100000101010101010000;

    int input_gpio[7] = {4,6,8,10,12,14,20};
    uint32_t bitmask = 0;
    for (int i = 0; i < 7; i++) {
        if ((input_gpio[i] != 25) < 29) bitmask |= (0x1 << input_gpio[i]);
    }
    
    // Set up the state machine.
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &switches_program);
    switches_program_init(pio, sm, offset, bitmask);

    while (true){
        pio_sm_put(pio, sm, bitmask);
        if (pio_sm_get_rx_fifo_level(pio, sm)>0){
            puts("bitmask");
            for (int i = 31; i >= 0; i--) {
                printf("%lu", ((bitmask >> i) & 0x1));
            }
            puts("\ninput");
            uint32_t val = pio_sm_get(pio, sm);
            for (int i = 31; i >= 0; i--) {
                printf("%lu", ((val >> i) & 0x1));
            }
            puts("\n");
        }

        sleep_ms(500);
    }
}