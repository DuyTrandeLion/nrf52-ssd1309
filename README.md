# nrf52-ssd1309
SSD1306/SSD1309 OLED driver for Nordic nRF52

Tested with nRF52-DK, nRF52833-DK and nRF52840-DK. 

The code is based on
[afiskon/stm32-ssd1306](https://github.com/afiskon/stm32-ssd1306) library
developed by Aleksander Alekseev ( [@afiskon](https://github.com/afiskon) ) in 2018.

**Text from the original author**
STM32 library for working with OLEDs based on SSD1306, SH1106 and SSD1309,
supports I2C and 4-wire SPI.

Tested on STM32F1, STM32F3, STM32F4, STM32L0, STM32L4 and STM32H7 MCUs, with 10 random displays from eBay.
Also this code is known to work with
[afiskon/fpga-ssd1306-to-vga](https://github.com/afiskon/fpga-ssd1306-to-vga).

Please see `examples` directory and `ssd1306/ssd1306.h` for more details.

The code is based on
[4ilo/ssd1306-stm32HAL](https://github.com/4ilo/ssd1306-stm32HAL) library
developed by Olivier Van den Eede ( [@4ilo](https://github.com/4ilo) ) in 2016.

See also:

* https://github.com/afiskon/stm32-ssd1351
* https://github.com/afiskon/stm32-st7735
* https://github.com/afiskon/stm32-ili9341
