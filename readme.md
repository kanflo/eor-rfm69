# RFM69 driver for ESP Open RTOS

This repository contains a port of André Heßling's [STM32 RFM69 driver](https://github.com/ahessling/RFM69-STM32) for the ESP8266. The included demo program sends the string "Hello World" over the air over and over again. RX has been tested also.

The driver depends on the SPI driver found in my repo, here is a short how-to:

```
git clone --recursive https://github.com/SuperHouse/esp-open-rtos.git
vi esp-open-rtos/include/ssid_config.h # Fix wifi credentials
git clone https://github.com/kanflo/eor-spi.git esp-open-rtos/extras/spi
git clone https://github.com/kanflo/eor-rfm69
cd eor-rfm69/example
export EOR_ROOT=/path/to/esp-open-rtos
make && make flash
```
