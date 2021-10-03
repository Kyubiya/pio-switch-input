#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "switches.pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

PIO pio;
dma_channel_config rx_c;
dma_channel_config tx_c;
int sm;
int dma_chan;
uint32_t bitmask;
uint32_t sw_data;

void dma_handler() {
    if (dma_channel_get_irq0_status(dma_chan)) {  
        // Clear the interrupt request.
        dma_channel_acknowledge_irq0(dma_chan);
        //Check rxf for data
        if (pio_sm_get_rx_fifo_level(pio, sm) > 0){
            //Has data, copy RXF to variable
            dma_channel_configure(dma_chan, &rx_c, &sw_data, &pio->rxf[sm], 1, true);
        } else {
            //RXF empty, put bitmask into TXF
            dma_channel_configure(dma_chan, &tx_c, &pio->txf[sm], &bitmask, 1, true);
        } 
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
    rx_c = dma_channel_get_default_config(dma_chan);
    tx_c = dma_channel_get_default_config(dma_chan);
    //Set DREQs
    channel_config_set_dreq(&rx_c, pio_get_dreq(pio, sm, false));
    channel_config_set_dreq(&tx_c, pio_get_dreq(pio, sm, true));
    //Disable increments
    channel_config_set_read_increment(&rx_c, false);
    channel_config_set_read_increment(&tx_c, false);
    // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_chan, true);
    // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    // Trigger first transfer
    dma_channel_configure(dma_chan, &tx_c, &pio->txf[sm], &bitmask, 1, true);

    while (true) {
        uint32_t temp = sw_data;
        for (int i = 31; i >= 0; i--) {
            printf("%lu", temp >> i & 0x1);
        }
        puts("");
    }
}