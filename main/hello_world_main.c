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

#define RGBLED_GPIO (CONFIG_RGBLED_GPIO)
#define STEP_INTERVAL (CONFIG_LED_STEP_INTERVAL)
#define LED_MIN_INTENSITY (CONFIG_LED_MIN_INTENSITY)
#define LED_MAX_INTENSITY (CONFIG_LED_MAX_INTENSITY)
#define LED_TOTAL_CYCLES (CONFIG_LED_TOTAL_CYCLES)
#define RESTART_DELAY (CONFIG_RESTART_DELAY)

#define TIME_INTERVAL_1S    (1000)  // in [ms]

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
        .strip_gpio_num = RGBLED_GPIO,
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
    bool led_red_off;
    bool led_green_off;
    bool led_blue_off;
    int cycles = LED_TOTAL_CYCLES;

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
    led_red_off = false; led_green_off = false; led_blue_off = false;

    ESP_LOGI(TAG,"========== Cycle %u begins", LED_TOTAL_CYCLES - (cycles - 1));
    do {
        color_led(red,green,blue);

        if (!led_red_off) {
            if (redDir) {
                ++red;
                if (red == (LED_MAX_INTENSITY)) {
                    redDir = false;
                    red = (LED_MAX_INTENSITY) - 1;
                    ESP_LOGI(TAG,"Red is fading down");
                }
            }
            else {
                --red;
                if (red == (LED_MIN_INTENSITY)) {
                    if (cycles == 0) {
                        led_red_off = true;
                    }
                    else {
                        redDir = true;
                        ESP_LOGI(TAG,"Red is rising up");
                    }
                }
            }
        }

        if (!led_green_off) {
            if (greenDir) {
                ++green;
                if (green == (LED_MAX_INTENSITY)) {
                    greenDir = false;
                    green = (LED_MAX_INTENSITY) - 1;
                    ESP_LOGI(TAG,"Green is fading down");
                }
            }
            else {
                --green;
                if (green == (LED_MIN_INTENSITY)) {
                    if (cycles == 0) {
                        led_green_off = true;
                    }
                    else {
                        greenDir = true;
                        ESP_LOGI(TAG,"Green is rising up");
                    }
                }
            }
        }   

        if (!led_blue_off) {
            if (blueDir) {
                ++blue;
                if (blue == (LED_MAX_INTENSITY)) {
                    blueDir = false;
                    blue = (LED_MAX_INTENSITY) - 1;
                    ESP_LOGI(TAG,"Blue is fading down");
                }
            }
            else {
                --blue;
                if (blue == (LED_MIN_INTENSITY)) {
                    if (--cycles == 0) {
                        led_blue_off = true;
                    }
                    else {
                        blueDir = true;
                        ESP_LOGI(TAG,"========== Cycle %u begins", LED_TOTAL_CYCLES - (cycles - 1));
                        ESP_LOGI(TAG,"Blue is rising up");
                    }
                }
            }
        }

        vTaskDelay((STEP_INTERVAL) / portTICK_PERIOD_MS);
    } while(!led_red_off || !led_green_off || !led_blue_off);

    color_led(red,green,blue);

    ESP_LOGI(TAG,"========== Lights are turned off. Good night.");

    for (int i = RESTART_DELAY; i >= 0; i--) {
        ESP_LOGI(TAG,"Restarting in %d seconds...", i);
        vTaskDelay(TIME_INTERVAL_1S / portTICK_PERIOD_MS);
    }

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
