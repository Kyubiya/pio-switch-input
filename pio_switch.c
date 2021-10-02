#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "switches.pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

PIO pio;
dma_channel_config c;
int sm;
int dma_chan;
uint32_t bitmask;
uint32_t sw_data;

void dma_handler() {
    // Clear the interrupt request.
    dma_hw->ints0 = 1u << dma_chan;
    //Check rxf for data
    if (pio_sm_get_rx_fifo_level(pio, sm) > 0){
        //Has data, copy RXF to variable
        channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));
        dma_channel_configure(dma_chan, &c, &sw_data, &pio->rxf[sm], 1, true);
    } else {
        //RXF empty, put bitmask into TXF
        channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
        dma_channel_configure(dma_chan, &c, &pio->txf[sm], &bitmask, 1, true);
    } 
}

int main() {
    stdio_init_all();

    //bitmask for inputs, 31->0
    //GPIO 31-29 are not available, and 25 is onboard led
    //bits indicated should always be 0
    //                     vvv    v
    //uint32_t bitmask = 0b00000000000100000101010101010000;

    int input_gpio[7] = {4,6,8,10,12,14,20};
    bitmask = 0;
    for (int i = 0; i < 7; i++) {
        if ((input_gpio[i] != 25) < 29) bitmask |= (0x1 << input_gpio[i]);
    }
    
    // Set up the state machine.
    pio = pio0;
    sm = 3;
    uint offset = pio_add_program(pio, &switches_program);
    switches_program_init(pio, sm, offset, bitmask);

    //DMA
    //Get dma channel
    dma_chan = dma_claim_unused_channel(true);
    //Configure
    c = dma_channel_get_default_config(dma_chan);
    //Do full transfer
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    //Disable increments
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);
    //Handler will set read/write, 1 transfer, do not start
    dma_channel_configure(dma_chan, &c, NULL, NULL, 1, false);
    // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_chan, true);
    // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    // Manually call the handler once, to trigger the first transfer
    dma_handler();

    while (true) {
        uint32_t temp = sw_data;
        for (int i = 31; i >= 0; i--) {
            printf("%lu", temp >> i & 0x1);
        }
        puts("");
    }
}