# spi_hc04

Uses the SPI bus to read distance from inexpensive HC-SR04 ultrasonic rangefinder modules on Linux single-board computers.

We do this by treating HC-SR04's `Trig` pin as `MOSI`, and the `Echo` pin as `MISO`.
We then issue an SPI transaction, writing 0xFF followed by a bunch of zeros.
Finally, we count the number of high bits in the response to measure the width of the echo pulse.

We assume that the HC-SR04 is the only thing connected to MOSI/MISO.
Chip-select functionality is left as an exercise for the reader.

## Wiring specifics for [CHIP](https://getchip.com/pages/chip)

The CHIP's GPIO pins are **not 5V tolerant**, so you will need a level converter, such as [this one from Sparkfun.](https://www.sparkfun.com/products/12009)

- CHIP's `VCC-5V` to HC-SR04's `VCC` and level converter's `HV`
- CHIP's `GND` to HC-SR04's `GND` and level converter's `GND`.
- CHIP's `VCC-3V3` to level converter's `LV`
- CHIP's `CSIHSYNC` (MOSI) to level converter's `LV1`
- HC-SR04's `Trig` to level converter's `HV1`
- CHIP's `CSIVSYNC` (MISO) to level converter's `LV2`
- HC-SR04's `Echo` to level converter's `LV2`
