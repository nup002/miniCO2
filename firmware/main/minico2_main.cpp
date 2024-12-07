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
// Project files
#include "types.h"
#include "scd40/scd40.h"
#include "led/led.h"
#include "controller/controller.h"
#include "ble/ble.h"
extern "C" {
#include "zigbee/zigbee.h"
#include "console/console.h"
}

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

// Make sure the required ZigBee source code files have been compiled
#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE in idf.py menuconfig to compile sensor (End Device) source code.
#endif

static constexpr  const char *MAIN_TAG = "main";

TaskHandle_t scd40_task_handle = NULL;
TaskHandle_t led_task_handle = NULL;
TaskHandle_t controller_task_handle = NULL;
TaskHandle_t ble_task_handle = NULL;
TaskHandle_t zigbee_task_handle = NULL;
TaskHandle_t console_task_handle = NULL;

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


//app_main must link to C code
#ifdef __cplusplus
extern "C" {
    void app_main(void);
}
#endif


void app_main(void)
{
    boot();  //Print ESP32 chip info to the serial port.
    
    // Init the queues
    struct SCD40measurement meas;
    QueueHandle_t measurements_queue = xQueueCreate(1, sizeof(meas));
    if (measurements_queue == 0){ESP_LOGE(MAIN_TAG, "Failed at creating measurements queue");}

    enum LED_STATES state;
    QueueHandle_t led_state_queue = xQueueCreate(1, sizeof(state));
    if (led_state_queue == 0){ESP_LOGE(MAIN_TAG, "Failed at creating LED state queue");}

    esp_err_t error;
    QueueHandle_t errors_queue = xQueueCreate(1, sizeof(error));
    if (errors_queue == 0){ESP_LOGE(MAIN_TAG, "Failed at creating error queue");}

    QueueHandle_t ble_queue = xQueueCreate(1, sizeof(meas));
    if (ble_queue == 0){ESP_LOGE(MAIN_TAG, "Failed at creating BLE queue");}

    QueueHandle_t zigbee_queue = xQueueCreate(1, sizeof(meas));
    if (zigbee_queue == 0){ESP_LOGE(MAIN_TAG, "Failed at creating ZigBee queue");}

    // Launch the LED task
    QueueHandle_t led_queues[] = {led_state_queue, errors_queue}; 
    xTaskCreate(led_task, "LED_task", configMINIMAL_STACK_SIZE * 8, led_queues, 5, &led_task_handle);

    // Launch the controller task
    QueueHandle_t controller_queues[] = {measurements_queue, led_state_queue, errors_queue, ble_queue, zigbee_queue};
    xTaskCreate(controller_task, "controller_task", configMINIMAL_STACK_SIZE * 8, controller_queues, 5, &controller_task_handle);

    // Launch the SCD40 sensor reader task
    QueueHandle_t scd40_queues[] = {measurements_queue, errors_queue}; 
    xTaskCreate(scd40_task, "SCD40_task", configMINIMAL_STACK_SIZE * 8, scd40_queues, 5, &scd40_task_handle);

    // Launch the BLE task
    QueueHandle_t ble_queues[] = {ble_queue, errors_queue}; 
    xTaskCreate(ble_task, "BLE_task", configMINIMAL_STACK_SIZE * 8, ble_queues, 5, &ble_task_handle);

    // Launch the ZigBee task
    QueueHandle_t zigbee_queues[] = {zigbee_queue, errors_queue}; 
    xTaskCreate(zigbee_task, "ZigBee_task", configMINIMAL_STACK_SIZE * 8, zigbee_queues, 5, &zigbee_task_handle);

    // Launch the console task
    xTaskCreate(console_task, "console_task", configMINIMAL_STACK_SIZE * 8, NULL, 5, &console_task_handle);
}