/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"

static const char *TAG = "led&hello";

#define BLINK_GPIO 48

static uint8_t s_led_state = 0;
static led_strip_handle_t led_strip;

static void color_led(uint32_t red, uint32_t green, uint32_t blue);

static void color_led(uint32_t red, uint32_t green, uint32_t blue)
{
    led_strip_set_pixel(led_strip, 0, red & 0xffu, green & 0xffu, blue & 0xffu);
    led_strip_refresh(led_strip);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void app_main(void)
{
    printf("Hello world!\n");
    printf("\nHey, I am imc and I am here in ESP32. Wait to see more from me!\n\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    /* Configure the peripheral according to the LED type */
    configure_led();

    uint32_t red, green, blue;
    red = 0u; green = 40u; blue = 80u;
    bool redDir, greenDir, blueDir;
    redDir = true; greenDir = true; blueDir = true;
    do {
        color_led(red,green,blue);

        if (redDir) {
            ++red;
            if (red == 100) {
                redDir = false;
                red = 99;
            }
        }
        else {
            --red;
            if (red == 0) {
                redDir = true;
            }
        }

        if (greenDir) {
            ++green;
            if (green == 100) {
                greenDir = false;
                green = 99;
            }
        }
        else {
            --green;
            if (green == 0) {
                greenDir = true;
            }
        }

        if (blueDir) {
            ++blue;
            if (blue == 100) {
                blueDir = false;
                blue = 99;
            }
        }
        else {
            --blue;
            if (blue == 0) {
                blueDir = true;
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    } while(1);


    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
