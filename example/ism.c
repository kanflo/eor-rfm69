/* 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Johan Kanflo (github.com/kanflo)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "espressif/esp_common.h"
#include "espressif/sdk_private.h"
#include <espressif/esp_misc.h>
#include <lwip/api.h>
#include <lwip/sockets.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <esp/spi.h>
#include <espressif/sdk_private.h>
#include <sys/types.h>
#include <unistd.h>

#include <rfm69.h>
#include <ssid_config.h>


#define delay_ms(t) vTaskDelay((t) / portTICK_PERIOD_MS)
#define systime_ms() (xTaskGetTickCount() * portTICK_PERIOD_MS)
#define TIME_MARKER "[%8u] "

// Set these depending on how yo hooked up the RFM69
#define RFM69_INT     (5)
#define RFM69_CS      (4)
#define RFM69_RESET   (0)
#define LED_STATUS   (15)
#define LED_ON        (0)
#define LED_OFF       (1)

static void set_led(uint32_t state)
{
#ifdef LED_STATUS
    gpio_write(LED_STATUS, state);
#endif // LED_STATUS
}

static void ism_task(void *p)
{
#ifdef RFM69_RESET
    rfm69_setResetPin(RFM69_RESET);
    rfm69_reset();
#endif // RFM69_RESET

    bool ok = rfm69_init(RFM69_CS, false); // false = RFM69W, true = RFM69HW

    if (!ok) {
        printf(TIME_MARKER "RFM69 init failed!\n", systime_ms());
    } else {
        printf(TIME_MARKER "RFM69 init ok\n", systime_ms());
        
//        gpio_enable(RFM69_INT, GPIO_INPUT);
//        rfm69_dumpRegisters();

//        rfm69_setResetPin(...);
//        rfm69_reset();

        // init RF module and put it to sleep
        rfm69_sleep();

        // set output power
        rfm69_setPowerDBm(10); // +10 dBm

        // enable CSMA/CA algorithm
        rfm69_setCSMA(true);

        (void) rfm69_setAESEncryption((void*) "sampleEncryptKey", 16);

        rfm69_setPowerLevel(0); // max 31

#if 0
        set_led(0);
        (void) rfm69_setMode(RFM69_MODE_TX); // Unmodulated carrier
        printf(TIME_MARKER "Carrier on\n", systime_ms());
#else
        while(1) {
            // send a packet and let RF module sleep
            char testdata[] = {1, 42, 0x00, 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
            set_led(LED_ON);
            rfm69_send(testdata, sizeof(testdata));
            set_led(LED_OFF);
            rfm69_sleep();

            printf(TIME_MARKER "Message sent\n", systime_ms());
            delay_ms(100);
        }
#endif
    }
    while(1) ;
}

void user_init(void)
{
    sdk_uart_div_modify(0, UART_CLK_FREQ / 115200);
    printf("\n\n\n");
    printf("SDK version:%s\n", sdk_system_get_sdk_version());
#ifdef LED_STATUS
        gpio_enable(LED_STATUS, GPIO_OUTPUT);
        set_led(LED_OFF);
#endif // LED_STATUS

    spi_init(1, SPI_MODE0, SPI_FREQ_DIV_1M, 1, SPI_LITTLE_ENDIAN, true); // init SPI module

#ifndef CONFIG_NO_WIFI
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };
    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
#endif // CONFIG_NO_WIFI

    xTaskCreate(&ism_task, "ism_task", 256, NULL, 2, NULL);
}
