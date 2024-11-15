#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "driver/gpio.h"
#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>
#include "led_strip.h"
#include "sdkconfig.h"
#include "../types.h"
#include "led.h"

static const char *TAG = "led";

static led_strip_handle_t led_strip;


esp_err_t initiate_led(void)
{
    ESP_LOGI(TAG, "Initializing the LED...");
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    
    ESP_LOGI(TAG, "1/2 - Creating new led_strip_handle_t object");
    ESP_RETURN_ON_ERROR(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip), TAG, "LED initialization failed");

    ESP_LOGI(TAG, "2/2 - Clearing LED");
    ESP_RETURN_ON_ERROR(led_strip_clear(led_strip), TAG, "LED clear failed");

    ESP_LOGI(TAG, "LED initialized succesfully");
    return ESP_OK;
}

void led_task(void *pvParameters)
{
    // Get the measurements queue from the pvParameters pointer
    QueueHandle_t led_state_queue = (QueueHandle_t)pvParameters;

    esp_err_t led_init_err = initiate_led();
    if (led_init_err){
        // Log the error and put MINICO2 into error state and return from the task
        ESP_ERROR_CHECK_WITHOUT_ABORT(led_init_err);
        return;
    }

    enum LED_STATES state;
    while (1){
        if (xQueueReceive(led_state_queue, &( state), (TickType_t) 10)){
            ESP_LOGD(TAG, "Received LED state %u", state);
        }
    }
}