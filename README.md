# PIO switch input
 Pi Pico input via PIO with bitmask

For Raspberry Pi Pico

Use PIO to retrieve input on pins marked with a 32bit mask.

Least significant bit in mask is pin 0 so bits go from pin 31 -> 0.
Pins 31,30,29 are not available and 25 is onboard LED and should be marked 0.

      vvv   v
    0b00000000000000000000000000000000
pins 31          ->                  0

Returns shortened value where the least significant bit is first marked pin.

Push bitmask into txr to poll inputs.

Switches on inputs are wired to ground.

Example:

	7 Switches on pins 4,6,8,10,12,14,20
	Bitmask to check pins
	0b00000000000100000101010101010000

	Returns:

	With all switches pressed
	0b00000000000000000000000001111111

	With only switch 20 pressed
	0b00000000000000000000000001000000