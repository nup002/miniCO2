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
#include <esp_err.h>
#include <esp_log.h>
#include "esp_system.h"
#include "scd4x.h"
#include "scd40/scd40.h"
#include "led/led.h"
#include "global_constants.h"

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

TaskHandle_t scd40_task_handle = NULL;
TaskHandle_t led_task_handle = NULL;

void boot(){
    // Tasks to take before anything else starts.

    printf("MiniCO2 reporting for duty.\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

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

    ESP_ERROR_CHECK(i2cdev_init());
}

void app_main(void)
{
    boot();  //Boot the MINICO2. Prints ESP32 chip info to the serial port.

    
    // Init the sensor measurements queue
    struct SCD40measurement meas;
    QueueHandle_t measurements_queue = xQueueCreate(1, sizeof((meas)));
    if (measurements_queue == 0)
    {
        ESP_LOGE(TAG, "Failed at creating measurements queue");
    }

    // Launch the SCD40 sensor reader task
    xTaskCreate(scd40_task, "SCD40_task", configMINIMAL_STACK_SIZE * 8, (void*)measurements_queue, 5, &scd40_task_handle);

    // Launch the LED controller task
    //xTaskCreate(led_task, "LED_task", configMINIMAL_STACK_SIZE * 8, (void*)measurements_queue, 5, &led_task_handle);
}
